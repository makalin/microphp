#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_usage(const char *program_name) {
    printf("micro-PHP Compiler (microphpc) v%s\n", MICROPHP_VERSION);
    printf("Usage: %s [options] <input_file> -o <output_file>\n", program_name);
    printf("\nOptions:\n");
    printf("  -o <file>     Output bytecode file (required)\n");
    printf("  -v            Verbose output\n");
    printf("  -h, --help    Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s script.php -o script.mbc\n", program_name);
    printf("  %s -v main.php -o main.mbc\n", program_name);
}

int read_file(const char *filename, char **content, size_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", filename);
        return -1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    *content = malloc(*size + 1);
    if (!*content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return -1;
    }
    
    // Read file content
    size_t bytes_read = fread(*content, 1, *size, file);
    fclose(file);
    
    if (bytes_read != *size) {
        fprintf(stderr, "Error: Failed to read file completely\n");
        free(*content);
        return -1;
    }
    
    (*content)[*size] = '\0';
    return 0;
}

int write_file(const char *filename, const uint8_t *data, size_t size) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", filename);
        return -1;
    }
    
    size_t bytes_written = fwrite(data, 1, size, file);
    fclose(file);
    
    if (bytes_written != size) {
        fprintf(stderr, "Error: Failed to write file completely\n");
        return -1;
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    bool verbose = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: Missing output file after -o\n");
                print_usage(argv[0]);
                return 1;
            }
        } else if (argv[i][0] != '-') {
            if (input_file) {
                fprintf(stderr, "Error: Multiple input files specified\n");
                print_usage(argv[0]);
                return 1;
            }
            input_file = argv[i];
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Check required arguments
    if (!input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (!output_file) {
        fprintf(stderr, "Error: No output file specified (-o)\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (verbose) {
        printf("micro-PHP Compiler v%s\n", MICROPHP_VERSION);
        printf("Input file: %s\n", input_file);
        printf("Output file: %s\n", output_file);
        printf("\n");
    }
    
    // Read input file
    char *source_code;
    size_t source_size;
    
    if (read_file(input_file, &source_code, &source_size) != 0) {
        return 1;
    }
    
    if (verbose) {
        printf("Source file size: %zu bytes\n", source_size);
        printf("\nCompiling...\n");
    }
    
    // Create compiler context
    compiler_context_t *ctx = compiler_create(source_code, source_size);
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create compiler context\n");
        free(source_code);
        return 1;
    }
    
    // Perform lexical analysis
    if (verbose) printf("Phase 1: Lexical analysis...\n");
    if (compiler_lex(ctx) != 0) {
        fprintf(stderr, "Error: Lexical analysis failed: %s\n", compiler_get_error(ctx));
        compiler_destroy(ctx);
        free(source_code);
        return 1;
    }
    
    if (verbose) {
        printf("  Generated %zu tokens\n", ctx->token_count);
    }
    
    // Perform parsing
    if (verbose) printf("Phase 2: Parsing...\n");
    if (compiler_parse(ctx) != 0) {
        fprintf(stderr, "Error: Parsing failed: %s\n", compiler_get_error(ctx));
        compiler_destroy(ctx);
        free(source_code);
        return 1;
    }
    
    if (verbose) printf("  AST created successfully\n");
    
    // Generate bytecode
    if (verbose) printf("Phase 3: Code generation...\n");
    uint8_t *bytecode;
    size_t bytecode_size;
    
    if (compiler_generate_bytecode(ctx, &bytecode, &bytecode_size) != 0) {
        fprintf(stderr, "Error: Code generation failed: %s\n", compiler_get_error(ctx));
        compiler_destroy(ctx);
        free(source_code);
        return 1;
    }
    
    if (verbose) {
        printf("  Generated %zu bytes of bytecode\n", bytecode_size);
    }
    
    // Write output file
    if (verbose) printf("Phase 4: Writing output...\n");
    if (write_file(output_file, bytecode, bytecode_size) != 0) {
        fprintf(stderr, "Error: Failed to write output file\n");
        free(bytecode);
        compiler_destroy(ctx);
        free(source_code);
        return 1;
    }
    
    if (verbose) printf("  Output written successfully\n");
    
    // Cleanup
    free(bytecode);
    compiler_destroy(ctx);
    free(source_code);
    
    if (verbose) {
        printf("\nCompilation completed successfully!\n");
        printf("Output: %s (%zu bytes)\n", output_file, bytecode_size);
    }
    
    return 0;
}
