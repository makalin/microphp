#ifndef MICROPHP_H
#define MICROPHP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Configuration
#define MICROPHP_VERSION "0.1.0"
#define MICROPHP_MAX_OPCODES 256
#define MICROPHP_MAX_CONSTANTS 1024
#define MICROPHP_MAX_FUNCTIONS 64
#define MICROPHP_MAX_LOCALS 128

// Zval types (PHP value types)
typedef enum {
    ZVAL_NULL = 0,
    ZVAL_BOOL,
    ZVAL_INT,
    ZVAL_FLOAT,
    ZVAL_STRING,
    ZVAL_ARRAY,
    ZVAL_OBJECT,
    ZVAL_CLOSURE,
    ZVAL_RESOURCE
} zval_type_t;

// Zval structure (PHP value)
typedef struct zval {
    zval_type_t type;
    union {
        bool bool_val;
        int64_t int_val;
        double float_val;
        struct {
            char *str;
            size_t len;
        } string_val;
        struct {
            struct zval *data;
            size_t size;
            size_t capacity;
        } array_val;
        struct {
            void *ptr;
            int type;
        } resource_val;
    } value;
} zval_t;

// Opcode types
typedef enum {
    OP_NOP = 0,
    OP_CONST,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_ASSIGN,
    OP_ASSIGN_ADD,
    OP_ASSIGN_SUB,
    OP_ASSIGN_MUL,
    OP_ASSIGN_DIV,
    OP_ASSIGN_MOD,
    OP_INC,
    OP_DEC,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_JMP,
    OP_JMPZ,
    OP_JMPNZ,
    OP_CALL,
    OP_RETURN,
    OP_POP,
    OP_DUP,
    OP_SWAP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_NEW_ARRAY,
    OP_ARRAY_GET,
    OP_ARRAY_SET,
    OP_STRING_CONCAT,
    OP_CAST_INT,
    OP_CAST_FLOAT,
    OP_CAST_STRING,
    OP_CAST_BOOL
} opcode_t;

// Instruction structure
typedef struct {
    opcode_t opcode;
    uint16_t operand1;
    uint16_t operand2;
} instruction_t;

// Function structure
typedef struct {
    char *name;
    size_t name_len;
    instruction_t *code;
    size_t code_size;
    size_t local_count;
    size_t param_count;
} function_t;

// Bytecode structure (MBC - Micro-PHP Bytecode)
typedef struct {
    char magic[4];           // "MBC\0"
    uint32_t version;        // Bytecode version
    uint32_t constant_count;
    zval_t *constants;
    uint32_t function_count;
    function_t *functions;
    uint32_t main_offset;    // Main function offset
} bytecode_t;

// VM context
typedef struct {
    bytecode_t *bytecode;
    zval_t *stack;
    size_t stack_size;
    size_t stack_top;
    zval_t *locals;
    size_t local_count;
    zval_t *globals;
    size_t global_count;
    instruction_t *pc;       // Program counter
    bool running;
    char *error_msg;
} vm_context_t;

// Core VM functions
vm_context_t* microphp_vm_create(void);
void microphp_vm_destroy(vm_context_t *vm);
int microphp_vm_load_bytecode(vm_context_t *vm, const uint8_t *data, size_t size);
int microphp_vm_run(vm_context_t *vm);
void microphp_vm_reset(vm_context_t *vm);

// Zval operations
zval_t microphp_zval_null(void);
zval_t microphp_zval_bool(bool value);
zval_t microphp_zval_int(int64_t value);
zval_t microphp_zval_float(double value);
zval_t microphp_zval_string(const char *str, size_t len);
zval_t microphp_zval_array(size_t initial_capacity);

void microphp_zval_destroy(zval_t *zval);
void microphp_zval_copy(zval_t *dest, const zval_t *src);
bool microphp_zval_equals(const zval_t *a, const zval_t *b);

// Array operations
int microphp_array_push(zval_t *array, const zval_t *value);
int microphp_array_get(const zval_t *array, size_t index, zval_t *result);
int microphp_array_set(zval_t *array, size_t index, const zval_t *value);
size_t microphp_array_size(const zval_t *array);

// String operations
zval_t microphp_string_concat(const zval_t *a, const zval_t *b);
int microphp_string_length(const zval_t *string);

// Built-in functions
zval_t microphp_builtin_print(const zval_t *args, size_t count);
zval_t microphp_builtin_sleep_ms(const zval_t *args, size_t count);
zval_t microphp_builtin_millis(const zval_t *args, size_t count);

// Error handling
const char* microphp_get_error(vm_context_t *vm);
void microphp_clear_error(vm_context_t *vm);

#ifdef __cplusplus
}
#endif

#endif // MICROPHP_H
