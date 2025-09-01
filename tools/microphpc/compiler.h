#ifndef MICROPHP_COMPILER_H
#define MICROPHP_COMPILER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Version information
#define MICROPHP_VERSION "0.1.0"

// Token types
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_ASSIGN,
    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_MULTIPLY_ASSIGN,
    TOKEN_DIVIDE_ASSIGN,
    TOKEN_MODULO_ASSIGN,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_THAN,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_THAN,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_QUESTION,
    TOKEN_COLON,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_FOREACH,
    TOKEN_FUNCTION,
    TOKEN_RETURN,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_ECHO,
    TOKEN_PRINT,
    TOKEN_SLEEP_MS,
    TOKEN_MILLIS
} token_type_t;

// Token structure
typedef struct {
    token_type_t type;
    char *value;
    size_t value_len;
    int line;
    int column;
} token_t;

// AST node types
typedef enum {
    AST_NODE_EXPRESSION = 0,
    AST_NODE_BINARY_OP,
    AST_NODE_UNARY_OP,
    AST_NODE_LITERAL,
    AST_NODE_IDENTIFIER,
    AST_NODE_FUNCTION_CALL,
    AST_NODE_ASSIGNMENT,
    AST_NODE_VARIABLE_DECLARATION,
    AST_NODE_IF_STATEMENT,
    AST_NODE_WHILE_STATEMENT,
    AST_NODE_FOR_STATEMENT,
    AST_NODE_BLOCK,
    AST_NODE_RETURN,
    AST_NODE_FUNCTION_DEFINITION,
    AST_NODE_CONTROL
} ast_node_type_t;

// AST node structure
typedef struct ast_node {
    ast_node_type_t type;
    struct ast_node *left;
    struct ast_node *right;
    union {
        // For literals
        struct {
            union {
                int64_t int_val;
                double float_val;
                char *string_val;
            } value;
            int literal_type;
        } literal;
        
        // For identifiers
        struct {
            char *name;
            size_t name_len;
        } identifier;
        
        // For function calls
        struct {
            char *name;
            size_t name_len;
            struct ast_node **arguments;
            size_t argument_count;
        } function_call;
        
        // For assignments
        struct {
            char *variable;
            size_t variable_len;
            struct ast_node *value;
        } assignment;
        
        // For control structures
        struct {
            struct ast_node *condition;
            struct ast_node *then_block;
            struct ast_node *else_block;
        } control;
        
        // For blocks
        struct {
            struct ast_node **statements;
            size_t statement_count;
        } block;
    } data;
} ast_node_t;

// Bytecode structure (MBC - Micro-PHP Bytecode)
typedef struct {
    char magic[4];           // "MBC\0"
    uint32_t version;        // Bytecode version
    uint32_t constant_count;
    void *constants;         // zval_t array
    uint32_t function_count;
    void *functions;         // function_t array
    uint32_t main_offset;    // Main function offset
} bytecode_t;

// Compiler context
typedef struct {
    char *source;
    size_t source_len;
    size_t position;
    int line;
    int column;
    token_t *tokens;
    size_t token_count;
    size_t token_capacity;
    ast_node_t *ast_root;
    char *error_msg;
    bool has_error;
} compiler_context_t;

// Compiler functions
compiler_context_t* compiler_create(const char *source, size_t source_len);
void compiler_destroy(compiler_context_t *ctx);

// Lexical analysis
int compiler_lex(compiler_context_t *ctx);
token_t* compiler_next_token(compiler_context_t *ctx);
void compiler_rewind_tokens(compiler_context_t *ctx);

// Parsing
int compiler_parse(compiler_context_t *ctx);
ast_node_t* compiler_parse_expression(compiler_context_t *ctx);
ast_node_t* compiler_parse_statement(compiler_context_t *ctx);
ast_node_t* compiler_parse_block(compiler_context_t *ctx);

// AST manipulation
ast_node_t* ast_create_node(ast_node_type_t type);
void ast_destroy_node(ast_node_t *node);
ast_node_t* ast_create_literal_int(int64_t value);
ast_node_t* ast_create_literal_float(double value);
ast_node_t* ast_create_literal_string(const char *value, size_t len);
ast_node_t* ast_create_identifier(const char *name, size_t len);
ast_node_t* ast_create_binary_op(ast_node_type_t op, ast_node_t *left, ast_node_t *right);
ast_node_t* ast_create_function_call(const char *name, size_t len, ast_node_t **args, size_t arg_count);

// Code generation
int compiler_generate_bytecode(compiler_context_t *ctx, uint8_t **output, size_t *output_size);

// Error handling
const char* compiler_get_error(compiler_context_t *ctx);
void compiler_set_error(compiler_context_t *ctx, const char *format, ...);

#endif // MICROPHP_COMPILER_H
