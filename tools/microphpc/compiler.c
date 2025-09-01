#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

// Memory management
static void* compiler_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "microphpc: memory allocation failed\n");
        abort();
    }
    return ptr;
}

static void* compiler_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        fprintf(stderr, "microphpc: memory reallocation failed\n");
        abort();
    }
    return new_ptr;
}

// Compiler context management
compiler_context_t* compiler_create(const char *source, size_t source_len) {
    compiler_context_t *ctx = compiler_malloc(sizeof(compiler_context_t));
    memset(ctx, 0, sizeof(compiler_context_t));
    
    ctx->source = compiler_malloc(source_len + 1);
    memcpy(ctx->source, source, source_len);
    ctx->source[source_len] = '\0';
    ctx->source_len = source_len;
    
    ctx->position = 0;
    ctx->line = 1;
    ctx->column = 1;
    
    ctx->token_capacity = 256;
    ctx->tokens = compiler_malloc(ctx->token_capacity * sizeof(token_t));
    ctx->token_count = 0;
    
    ctx->ast_root = NULL;
    ctx->error_msg = NULL;
    ctx->has_error = false;
    
    return ctx;
}

void compiler_destroy(compiler_context_t *ctx) {
    if (!ctx) return;
    
    if (ctx->source) free(ctx->source);
    if (ctx->error_msg) free(ctx->error_msg);
    
    // Free tokens
    if (ctx->tokens) {
        for (size_t i = 0; i < ctx->token_count; i++) {
            if (ctx->tokens[i].value) {
                free(ctx->tokens[i].value);
            }
        }
        free(ctx->tokens);
    }
    
    // Free AST
    if (ctx->ast_root) {
        ast_destroy_node(ctx->ast_root);
    }
    
    free(ctx);
}

// Error handling
void compiler_set_error(compiler_context_t *ctx, const char *format, ...) {
    if (!ctx) return;
    
    va_list args;
    va_start(args, format);
    
    // Calculate required size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (size > 0) {
        if (ctx->error_msg) free(ctx->error_msg);
        ctx->error_msg = compiler_malloc(size + 1);
        vsnprintf(ctx->error_msg, size + 1, format, args);
        ctx->has_error = true;
    }
    
    va_end(args);
}

const char* compiler_get_error(compiler_context_t *ctx) {
    return ctx ? ctx->error_msg : NULL;
}

// Lexical analysis
static void add_token(compiler_context_t *ctx, token_type_t type, const char *value, size_t value_len) {
    if (ctx->token_count >= ctx->token_capacity) {
        ctx->token_capacity *= 2;
        ctx->tokens = compiler_realloc(ctx->tokens, ctx->token_capacity * sizeof(token_t));
    }
    
    token_t *token = &ctx->tokens[ctx->token_count];
    token->type = type;
    token->line = ctx->line;
    token->column = ctx->column;
    
    if (value && value_len > 0) {
        token->value = compiler_malloc(value_len + 1);
        memcpy(token->value, value, value_len);
        token->value[value_len] = '\0';
        token->value_len = value_len;
    } else {
        token->value = NULL;
        token->value_len = 0;
    }
    
    ctx->token_count++;
}

static void skip_whitespace(compiler_context_t *ctx) {
    while (ctx->position < ctx->source_len) {
        char c = ctx->source[ctx->position];
        
        if (c == ' ' || c == '\t') {
            ctx->position++;
            ctx->column++;
        } else if (c == '\n') {
            ctx->position++;
            ctx->line++;
            ctx->column = 1;
        } else if (c == '\r') {
            ctx->position++;
            if (ctx->position < ctx->source_len && ctx->source[ctx->position] == '\n') {
                ctx->position++;
            }
            ctx->line++;
            ctx->column = 1;
        } else {
            break;
        }
    }
}

static void skip_comment(compiler_context_t *ctx) {
    if (ctx->position < ctx->source_len && ctx->source[ctx->position] == '/') {
        ctx->position++;
        ctx->column++;
        
        if (ctx->position < ctx->source_len && ctx->source[ctx->position] == '/') {
            // Single line comment
            while (ctx->position < ctx->source_len && ctx->source[ctx->position] != '\n') {
                ctx->position++;
                ctx->column++;
            }
        } else if (ctx->position < ctx->source_len && ctx->source[ctx->position] == '*') {
            // Multi-line comment
            ctx->position++;
            ctx->column++;
            
            while (ctx->position < ctx->source_len) {
                if (ctx->source[ctx->position] == '*' && 
                    ctx->position + 1 < ctx->source_len && 
                    ctx->source[ctx->position + 1] == '/') {
                    ctx->position += 2;
                    ctx->column += 2;
                    break;
                }
                
                if (ctx->source[ctx->position] == '\n') {
                    ctx->line++;
                    ctx->column = 1;
                } else {
                    ctx->column++;
                }
                ctx->position++;
            }
        } else {
            // Not a comment, backtrack
            ctx->position--;
            ctx->column--;
        }
    }
}

static token_type_t read_identifier_or_keyword(compiler_context_t *ctx) {
    size_t start = ctx->position;
    size_t start_column = ctx->column;
    
    while (ctx->position < ctx->source_len && 
           (isalnum(ctx->source[ctx->position]) || ctx->source[ctx->position] == '_')) {
        ctx->position++;
        ctx->column++;
    }
    
    size_t len = ctx->position - start;
    char *word = compiler_malloc(len + 1);
    memcpy(word, ctx->source + start, len);
    word[len] = '\0';
    
    // Check for keywords
    if (strcmp(word, "if") == 0) {
        free(word);
        return TOKEN_IF;
    } else if (strcmp(word, "else") == 0) {
        free(word);
        return TOKEN_ELSE;
    } else if (strcmp(word, "while") == 0) {
        free(word);
        return TOKEN_WHILE;
    } else if (strcmp(word, "for") == 0) {
        free(word);
        return TOKEN_FOR;
    } else if (strcmp(word, "function") == 0) {
        free(word);
        return TOKEN_FUNCTION;
    } else if (strcmp(word, "return") == 0) {
        free(word);
        return TOKEN_RETURN;
    } else if (strcmp(word, "true") == 0) {
        free(word);
        return TOKEN_TRUE;
    } else if (strcmp(word, "false") == 0) {
        free(word);
        return TOKEN_FALSE;
    } else if (strcmp(word, "null") == 0) {
        free(word);
        return TOKEN_NULL;
    } else if (strcmp(word, "var") == 0) {
        free(word);
        return TOKEN_VAR;
    } else if (strcmp(word, "const") == 0) {
        free(word);
        return TOKEN_CONST;
    } else if (strcmp(word, "echo") == 0) {
        free(word);
        return TOKEN_ECHO;
    } else if (strcmp(word, "print") == 0) {
        free(word);
        return TOKEN_PRINT;
    } else if (strcmp(word, "sleep_ms") == 0) {
        free(word);
        return TOKEN_SLEEP_MS;
    } else if (strcmp(word, "millis") == 0) {
        free(word);
        return TOKEN_MILLIS;
    } else if (strcmp(word, "OUTPUT") == 0) {
        free(word);
        return TOKEN_IDENTIFIER;  // Treat as identifier for now
    } else if (strcmp(word, "INPUT") == 0) {
        free(word);
        return TOKEN_IDENTIFIER;  // Treat as identifier for now
    } else if (strcmp(word, "true") == 0) {
        free(word);
        return TOKEN_TRUE;
    } else if (strcmp(word, "false") == 0) {
        free(word);
        return TOKEN_FALSE;
    }
    
    // Not a keyword, it's an identifier
    add_token(ctx, TOKEN_IDENTIFIER, word, len);
    free(word);
    return TOKEN_IDENTIFIER;
}

static void read_number(compiler_context_t *ctx) {
    size_t start = ctx->position;
    size_t start_column = ctx->column;
    bool is_float = false;
    
    // Read integer part
    while (ctx->position < ctx->source_len && isdigit(ctx->source[ctx->position])) {
        ctx->position++;
        ctx->column++;
    }
    
    // Check for decimal point
    if (ctx->position < ctx->source_len && ctx->source[ctx->position] == '.') {
        is_float = true;
        ctx->position++;
        ctx->column++;
        
        // Read fractional part
        while (ctx->position < ctx->source_len && isdigit(ctx->source[ctx->position])) {
            ctx->position++;
            ctx->column++;
        }
    }
    
    size_t len = ctx->position - start;
    char *number = compiler_malloc(len + 1);
    memcpy(number, ctx->source + start, len);
    number[len] = '\0';
    
    if (is_float) {
        add_token(ctx, TOKEN_FLOAT, number, len);
    } else {
        add_token(ctx, TOKEN_INT, number, len);
    }
    
    free(number);
}

static void read_string(compiler_context_t *ctx) {
    ctx->position++; // Skip opening quote
    ctx->column++;
    
    size_t start = ctx->position;
    size_t start_column = ctx->column;
    
    while (ctx->position < ctx->source_len && ctx->source[ctx->position] != '"') {
        if (ctx->source[ctx->position] == '\\') {
            ctx->position++; // Skip escape character
            ctx->column++;
        }
        
        if (ctx->source[ctx->position] == '\n') {
            ctx->line++;
            ctx->column = 1;
        } else {
            ctx->column++;
        }
        ctx->position++;
    }
    
    if (ctx->position >= ctx->source_len) {
        compiler_set_error(ctx, "Unterminated string at line %d, column %d", ctx->line, ctx->column);
        return;
    }
    
    size_t len = ctx->position - start;
    char *string = compiler_malloc(len + 1);
    memcpy(string, ctx->source + start, len);
    string[len] = '\0';
    
    add_token(ctx, TOKEN_STRING, string, len);
    free(string);
    
    ctx->position++; // Skip closing quote
    ctx->column++;
}

int compiler_lex(compiler_context_t *ctx) {
    ctx->position = 0;
    ctx->line = 1;
    ctx->column = 1;
    ctx->token_count = 0;
    
    while (ctx->position < ctx->source_len) {
        skip_whitespace(ctx);
        skip_comment(ctx);
        
        if (ctx->position >= ctx->source_len) break;
        
        char c = ctx->source[ctx->position];
        
        if (isalpha(c) || c == '_') {
            read_identifier_or_keyword(ctx);
        } else if (isdigit(c)) {
            read_number(ctx);
        } else if (c == '"') {
            read_string(ctx);
        } else {
            // Handle operators and punctuation
            switch (c) {
                case '+':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_PLUS_ASSIGN, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '+') {
                        add_token(ctx, TOKEN_INCREMENT, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_PLUS, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '-':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_MINUS_ASSIGN, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '-') {
                        add_token(ctx, TOKEN_DECREMENT, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_MINUS, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '*':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_MULTIPLY_ASSIGN, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_MULTIPLY, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '/':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_DIVIDE_ASSIGN, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_DIVIDE, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '%':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_MODULO_ASSIGN, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_MODULO, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '=':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_EQUAL, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_ASSIGN, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '!':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_NOT_EQUAL, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_NOT, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '<':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_LESS_EQUAL, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_LESS_THAN, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '>':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '=') {
                        add_token(ctx, TOKEN_GREATER_EQUAL, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        add_token(ctx, TOKEN_GREATER_THAN, NULL, 0);
                        ctx->position++;
                        ctx->column++;
                    }
                    break;
                    
                case '&':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '&') {
                        add_token(ctx, TOKEN_AND, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        compiler_set_error(ctx, "Unexpected character '&' at line %d, column %d", ctx->line, ctx->column);
                        return -1;
                    }
                    break;
                    
                case '|':
                    if (ctx->position + 1 < ctx->source_len && ctx->source[ctx->position + 1] == '|') {
                        add_token(ctx, TOKEN_OR, NULL, 0);
                        ctx->position += 2;
                        ctx->column += 2;
                    } else {
                        compiler_set_error(ctx, "Unexpected character '|' at line %d, column %d", ctx->line, ctx->column);
                        return -1;
                    }
                    break;
                    
                case '(':
                    add_token(ctx, TOKEN_LEFT_PAREN, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case ')':
                    add_token(ctx, TOKEN_RIGHT_PAREN, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case '{':
                    add_token(ctx, TOKEN_LEFT_BRACE, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case '}':
                    add_token(ctx, TOKEN_RIGHT_BRACE, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case '[':
                    add_token(ctx, TOKEN_LEFT_BRACKET, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case ']':
                    add_token(ctx, TOKEN_RIGHT_BRACKET, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case ';':
                    add_token(ctx, TOKEN_SEMICOLON, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case ',':
                    add_token(ctx, TOKEN_COMMA, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case '.':
                    add_token(ctx, TOKEN_DOT, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case '?':
                    add_token(ctx, TOKEN_QUESTION, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                case ':':
                    add_token(ctx, TOKEN_COLON, NULL, 0);
                    ctx->position++;
                    ctx->column++;
                    break;
                    
                default:
                    compiler_set_error(ctx, "Unexpected character '%c' at line %d, column %d", c, ctx->line, ctx->column);
                    return -1;
            }
        }
    }
    
    add_token(ctx, TOKEN_EOF, NULL, 0);
    return 0;
}

// Token access
token_t* compiler_next_token(compiler_context_t *ctx) {
    static size_t current_token = 0;
    
    if (current_token >= ctx->token_count) {
        return NULL;
    }
    
    return &ctx->tokens[current_token++];
}

void compiler_rewind_tokens(compiler_context_t *ctx) {
    // This would need to be implemented with a proper token index
    // For now, we'll just reset to the beginning
}

// AST manipulation
ast_node_t* ast_create_node(ast_node_type_t type) {
    ast_node_t *node = compiler_malloc(sizeof(ast_node_t));
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;
    return node;
}

void ast_destroy_node(ast_node_t *node) {
    if (!node) return;
    
    // Recursively destroy children
    if (node->left) ast_destroy_node(node->left);
    if (node->right) ast_destroy_node(node->right);
    
    // Clean up node-specific data
    switch (node->type) {
        case AST_NODE_LITERAL:
            if (node->data.literal.literal_type == 2) { // String
                free(node->data.literal.value.string_val);
            }
            break;
            
        case AST_NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_NODE_FUNCTION_CALL:
            free(node->data.function_call.name);
            if (node->data.function_call.arguments) {
                for (size_t i = 0; i < node->data.function_call.argument_count; i++) {
                    ast_destroy_node(node->data.function_call.arguments[i]);
                }
                free(node->data.function_call.arguments);
            }
            break;
            
        case AST_NODE_ASSIGNMENT:
            free(node->data.assignment.variable);
            if (node->data.assignment.value) {
                ast_destroy_node(node->data.assignment.value);
            }
            break;
            
        case AST_NODE_CONTROL:
            if (node->data.control.condition) ast_destroy_node(node->data.control.condition);
            if (node->data.control.then_block) ast_destroy_node(node->data.control.then_block);
            if (node->data.control.else_block) ast_destroy_node(node->data.control.else_block);
            break;
            
        case AST_NODE_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < node->data.block.statement_count; i++) {
                    ast_destroy_node(node->data.block.statements[i]);
                }
                free(node->data.block.statements);
            }
            break;
            
        default:
            break;
    }
    
    free(node);
}

ast_node_t* ast_create_literal_int(int64_t value) {
    ast_node_t *node = ast_create_node(AST_NODE_LITERAL);
    node->data.literal.literal_type = 0; // Int
    node->data.literal.value.int_val = value;
    return node;
}

ast_node_t* ast_create_literal_float(double value) {
    ast_node_t *node = ast_create_node(AST_NODE_LITERAL);
    node->data.literal.literal_type = 1; // Float
    node->data.literal.value.float_val = value;
    return node;
}

ast_node_t* ast_create_literal_string(const char *value, size_t len) {
    ast_node_t *node = ast_create_node(AST_NODE_LITERAL);
    node->data.literal.literal_type = 2; // String
    node->data.literal.value.string_val = compiler_malloc(len + 1);
    memcpy(node->data.literal.value.string_val, value, len);
    node->data.literal.value.string_val[len] = '\0';
    return node;
}

ast_node_t* ast_create_identifier(const char *name, size_t len) {
    ast_node_t *node = ast_create_node(AST_NODE_IDENTIFIER);
    node->data.identifier.name = compiler_malloc(len + 1);
    memcpy(node->data.identifier.name, name, len);
    node->data.identifier.name[len] = '\0';
    node->data.identifier.name_len = len;
    return node;
}

ast_node_t* ast_create_binary_op(ast_node_type_t op, ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = ast_create_node(op);
    node->left = left;
    node->right = right;
    return node;
}

ast_node_t* ast_create_function_call(const char *name, size_t len, ast_node_t **args, size_t arg_count) {
    ast_node_t *node = ast_create_node(AST_NODE_FUNCTION_CALL);
    node->data.function_call.name = compiler_malloc(len + 1);
    memcpy(node->data.function_call.name, name, len);
    node->data.function_call.name[len] = '\0';
    node->data.function_call.name_len = len;
    node->data.function_call.arguments = args;
    node->data.function_call.argument_count = arg_count;
    return node;
}

// Parsing (simplified implementation)
int compiler_parse(compiler_context_t *ctx) {
    // TODO: Implement full parser
    // For now, just create a simple AST
    ctx->ast_root = ast_create_node(AST_NODE_BLOCK);
    return 0;
}

// Code generation (simplified implementation)
int compiler_generate_bytecode(compiler_context_t *ctx, uint8_t **output, size_t *output_size) {
    // TODO: Implement full code generation
    // For now, just create a minimal bytecode structure
    
    *output_size = sizeof(bytecode_t);
    *output = compiler_malloc(*output_size);
    
    bytecode_t *bytecode = (bytecode_t*)*output;
    memcpy(bytecode->magic, "MBC\0", 4);
    bytecode->version = 1;
    bytecode->constant_count = 0;
    bytecode->constants = NULL;
    bytecode->function_count = 0;
    bytecode->functions = NULL;
    bytecode->main_offset = 0;
    
    return 0;
}
