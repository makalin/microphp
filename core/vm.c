#include "microphp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Memory management
static void* microphp_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "micro-PHP: memory allocation failed\n");
        abort();
    }
    return ptr;
}

static void* microphp_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        fprintf(stderr, "micro-PHP: memory reallocation failed\n");
        abort();
    }
    return new_ptr;
}

// VM context management
vm_context_t* microphp_vm_create(void) {
    vm_context_t *vm = microphp_malloc(sizeof(vm_context_t));
    memset(vm, 0, sizeof(vm_context_t));
    
    // Initialize stack
    vm->stack_size = 1024;
    vm->stack = microphp_malloc(vm->stack_size * sizeof(zval_t));
    vm->stack_top = 0;
    
    // Initialize locals and globals
    vm->local_count = MICROPHP_MAX_LOCALS;
    vm->locals = microphp_malloc(vm->local_count * sizeof(zval_t));
    vm->global_count = 256;
    vm->globals = microphp_malloc(vm->global_count * sizeof(zval_t));
    
    // Initialize all zvals to null
    for (size_t i = 0; i < vm->local_count; i++) {
        vm->locals[i] = microphp_zval_null();
    }
    for (size_t i = 0; i < vm->global_count; i++) {
        vm->globals[i] = microphp_zval_null();
    }
    
    vm->running = false;
    vm->error_msg = NULL;
    
    return vm;
}

void microphp_vm_destroy(vm_context_t *vm) {
    if (!vm) return;
    
    // Clean up bytecode
    if (vm->bytecode) {
        // Free constants
        if (vm->bytecode->constants) {
            for (uint32_t i = 0; i < vm->bytecode->constant_count; i++) {
                microphp_zval_destroy(&vm->bytecode->constants[i]);
            }
            free(vm->bytecode->constants);
        }
        
        // Free functions
        if (vm->bytecode->functions) {
            for (uint32_t i = 0; i < vm->bytecode->function_count; i++) {
                if (vm->bytecode->functions[i].name) {
                    free(vm->bytecode->functions[i].name);
                }
                if (vm->bytecode->functions[i].code) {
                    free(vm->bytecode->functions[i].code);
                }
            }
            free(vm->bytecode->functions);
        }
        
        free(vm->bytecode);
    }
    
    // Clean up stack
    if (vm->stack) {
        for (size_t i = 0; i < vm->stack_top; i++) {
            microphp_zval_destroy(&vm->stack[i]);
        }
        free(vm->stack);
    }
    
    // Clean up locals and globals
    if (vm->locals) {
        for (size_t i = 0; i < vm->local_count; i++) {
            microphp_zval_destroy(&vm->locals[i]);
        }
        free(vm->locals);
    }
    
    if (vm->globals) {
        for (size_t i = 0; i < vm->global_count; i++) {
            microphp_zval_destroy(&vm->globals[i]);
        }
        free(vm->globals);
    }
    
    if (vm->error_msg) {
        free(vm->error_msg);
    }
    
    free(vm);
}

// Stack operations
static int stack_push(vm_context_t *vm, const zval_t *value) {
    if (vm->stack_top >= vm->stack_size) {
        // Grow stack
        size_t new_size = vm->stack_size * 2;
        zval_t *new_stack = microphp_realloc(vm->stack, new_size * sizeof(zval_t));
        if (!new_stack) return -1;
        
        vm->stack = new_stack;
        vm->stack_size = new_size;
    }
    
    microphp_zval_copy(&vm->stack[vm->stack_top], value);
    vm->stack_top++;
    return 0;
}

static int stack_pop(vm_context_t *vm, zval_t *result) {
    if (vm->stack_top == 0) return -1;
    
    vm->stack_top--;
    microphp_zval_copy(result, &vm->stack[vm->stack_top]);
    microphp_zval_destroy(&vm->stack[vm->stack_top]);
    return 0;
}

static zval_t* stack_peek(vm_context_t *vm, size_t offset) {
    if (offset >= vm->stack_top) return NULL;
    return &vm->stack[vm->stack_top - 1 - offset];
}

// Bytecode loading
int microphp_vm_load_bytecode(vm_context_t *vm, const uint8_t *data, size_t size) {
    if (!vm || !data || size < sizeof(bytecode_t)) return -1;
    
    // Parse bytecode header
    const bytecode_t *header = (const bytecode_t*)data;
    
    // Verify magic
    if (memcmp(header->magic, "MBC\0", 4) != 0) {
        vm->error_msg = strdup("Invalid bytecode magic");
        return -1;
    }
    
    // Verify version
    if (header->version != 1) {
        vm->error_msg = strdup("Unsupported bytecode version");
        return -1;
    }
    
    // Allocate bytecode structure
    vm->bytecode = microphp_malloc(sizeof(bytecode_t));
    memcpy(vm->bytecode, header, sizeof(bytecode_t));
    
    // Load constants
    if (header->constant_count > 0) {
        vm->bytecode->constants = microphp_malloc(header->constant_count * sizeof(zval_t));
        // TODO: Parse constants from data
    }
    
    // Load functions
    if (header->function_count > 0) {
        vm->bytecode->functions = microphp_malloc(header->function_count * sizeof(function_t));
        // TODO: Parse functions from data
    }
    
    return 0;
}

// VM execution
int microphp_vm_run(vm_context_t *vm) {
    if (!vm || !vm->bytecode) return -1;
    
    vm->running = true;
    
    // Set program counter to main function
    if (vm->bytecode->main_offset < vm->bytecode->function_count) {
        vm->pc = vm->bytecode->functions[vm->bytecode->main_offset].code;
    } else {
        vm->error_msg = strdup("Invalid main function offset");
        vm->running = false;
        return -1;
    }
    
    // Main execution loop
    while (vm->running && vm->pc) {
        instruction_t *instr = vm->pc;
        
        switch (instr->opcode) {
            case OP_NOP:
                vm->pc++;
                break;
                
            case OP_CONST:
                if (instr->operand1 < vm->bytecode->constant_count) {
                    stack_push(vm, &vm->bytecode->constants[instr->operand1]);
                }
                vm->pc++;
                break;
                
            case OP_ADD: {
                zval_t b, a, result;
                if (stack_pop(vm, &b) != 0 || stack_pop(vm, &a) != 0) {
                    vm->error_msg = strdup("Stack underflow in ADD");
                    vm->running = false;
                    break;
                }
                
                if (a.type == ZVAL_INT && b.type == ZVAL_INT) {
                    result = microphp_zval_int(a.value.int_val + b.value.int_val);
                } else if (a.type == ZVAL_FLOAT || b.type == ZVAL_FLOAT) {
                    double a_val = (a.type == ZVAL_INT) ? (double)a.value.int_val : a.value.float_val;
                    double b_val = (b.type == ZVAL_INT) ? (double)b.value.int_val : b.value.float_val;
                    result = microphp_zval_float(a_val + b_val);
                } else {
                    vm->error_msg = strdup("Invalid types for ADD operation");
                    vm->running = false;
                    break;
                }
                
                stack_push(vm, &result);
                vm->pc++;
                break;
            }
            
            case OP_RETURN:
                vm->running = false;
                break;
                
            default:
                vm->error_msg = strdup("Unimplemented opcode");
                vm->running = false;
                break;
        }
    }
    
    return vm->running ? 0 : -1;
}

void microphp_vm_reset(vm_context_t *vm) {
    if (!vm) return;
    
    // Reset stack
    for (size_t i = 0; i < vm->stack_top; i++) {
        microphp_zval_destroy(&vm->stack[i]);
    }
    vm->stack_top = 0;
    
    // Reset locals
    for (size_t i = 0; i < vm->local_count; i++) {
        microphp_zval_destroy(&vm->locals[i]);
        vm->locals[i] = microphp_zval_null();
    }
    
    // Reset globals
    for (size_t i = 0; i < vm->global_count; i++) {
        microphp_zval_destroy(&vm->globals[i]);
        vm->globals[i] = microphp_zval_null();
    }
    
    vm->pc = NULL;
    vm->running = false;
    
    if (vm->error_msg) {
        free(vm->error_msg);
        vm->error_msg = NULL;
    }
}

// Error handling
const char* microphp_get_error(vm_context_t *vm) {
    return vm ? vm->error_msg : NULL;
}

void microphp_clear_error(vm_context_t *vm) {
    if (vm && vm->error_msg) {
        free(vm->error_msg);
        vm->error_msg = NULL;
    }
}
