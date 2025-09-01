#include "microphp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Zval creation
zval_t microphp_zval_null(void) {
    zval_t zval;
    zval.type = ZVAL_NULL;
    return zval;
}

zval_t microphp_zval_bool(bool value) {
    zval_t zval;
    zval.type = ZVAL_BOOL;
    zval.value.bool_val = value;
    return zval;
}

zval_t microphp_zval_int(int64_t value) {
    zval_t zval;
    zval.type = ZVAL_INT;
    zval.value.int_val = value;
    return zval;
}

zval_t microphp_zval_float(double value) {
    zval_t zval;
    zval.type = ZVAL_FLOAT;
    zval.value.float_val = value;
    return zval;
}

zval_t microphp_zval_string(const char *str, size_t len) {
    zval_t zval;
    zval.type = ZVAL_STRING;
    
    if (str && len > 0) {
        zval.value.string_val.str = malloc(len + 1);
        if (zval.value.string_val.str) {
            memcpy(zval.value.string_val.str, str, len);
            zval.value.string_val.str[len] = '\0';
            zval.value.string_val.len = len;
        } else {
            zval.type = ZVAL_NULL;
            zval.value.string_val.str = NULL;
            zval.value.string_val.len = 0;
        }
    } else {
        zval.value.string_val.str = NULL;
        zval.value.string_val.len = 0;
    }
    
    return zval;
}

zval_t microphp_zval_array(size_t initial_capacity) {
    zval_t zval;
    zval.type = ZVAL_ARRAY;
    
    if (initial_capacity > 0) {
        zval.value.array_val.data = malloc(initial_capacity * sizeof(zval_t));
        if (zval.value.array_val.data) {
            zval.value.array_val.size = 0;
            zval.value.array_val.capacity = initial_capacity;
            
            // Initialize all elements to null
            for (size_t i = 0; i < initial_capacity; i++) {
                zval.value.array_val.data[i] = microphp_zval_null();
            }
        } else {
            zval.type = ZVAL_NULL;
            zval.value.array_val.data = NULL;
            zval.value.array_val.size = 0;
            zval.value.array_val.capacity = 0;
        }
    } else {
        zval.value.array_val.data = NULL;
        zval.value.array_val.size = 0;
        zval.value.array_val.capacity = 0;
    }
    
    return zval;
}

// Zval destruction
void microphp_zval_destroy(zval_t *zval) {
    if (!zval) return;
    
    switch (zval->type) {
        case ZVAL_STRING:
            if (zval->value.string_val.str) {
                free(zval->value.string_val.str);
                zval->value.string_val.str = NULL;
            }
            zval->value.string_val.len = 0;
            break;
            
        case ZVAL_ARRAY:
            if (zval->value.array_val.data) {
                for (size_t i = 0; i < zval->value.array_val.size; i++) {
                    microphp_zval_destroy(&zval->value.array_val.data[i]);
                }
                free(zval->value.array_val.data);
                zval->value.array_val.data = NULL;
            }
            zval->value.array_val.size = 0;
            zval->value.array_val.capacity = 0;
            break;
            
        case ZVAL_OBJECT:
        case ZVAL_CLOSURE:
        case ZVAL_RESOURCE:
            // TODO: Implement proper cleanup for these types
            break;
            
        default:
            break;
    }
    
    zval->type = ZVAL_NULL;
}

// Zval copying
void microphp_zval_copy(zval_t *dest, const zval_t *src) {
    if (!dest || !src) return;
    
    // Clean up destination first
    microphp_zval_destroy(dest);
    
    // Copy basic structure
    dest->type = src->type;
    
    switch (src->type) {
        case ZVAL_NULL:
            break;
            
        case ZVAL_BOOL:
            dest->value.bool_val = src->value.bool_val;
            break;
            
        case ZVAL_INT:
            dest->value.int_val = src->value.int_val;
            break;
            
        case ZVAL_FLOAT:
            dest->value.float_val = src->value.float_val;
            break;
            
        case ZVAL_STRING:
            if (src->value.string_val.str && src->value.string_val.len > 0) {
                dest->value.string_val.str = malloc(src->value.string_val.len + 1);
                if (dest->value.string_val.str) {
                    memcpy(dest->value.string_val.str, src->value.string_val.str, src->value.string_val.len);
                    dest->value.string_val.str[src->value.string_val.len] = '\0';
                    dest->value.string_val.len = src->value.string_val.len;
                } else {
                    dest->type = ZVAL_NULL;
                }
            } else {
                dest->value.string_val.str = NULL;
                dest->value.string_val.len = 0;
            }
            break;
            
        case ZVAL_ARRAY:
            if (src->value.array_val.data && src->value.array_val.capacity > 0) {
                dest->value.array_val.data = malloc(src->value.array_val.capacity * sizeof(zval_t));
                if (dest->value.array_val.data) {
                    dest->value.array_val.capacity = src->value.array_val.capacity;
                    dest->value.array_val.size = src->value.array_val.size;
                    
                    // Copy array elements
                    for (size_t i = 0; i < src->value.array_val.size; i++) {
                        microphp_zval_copy(&dest->value.array_val.data[i], &src->value.array_val.data[i]);
                    }
                    
                    // Initialize remaining elements to null
                    for (size_t i = src->value.array_val.size; i < src->value.array_val.capacity; i++) {
                        dest->value.array_val.data[i] = microphp_zval_null();
                    }
                } else {
                    dest->type = ZVAL_NULL;
                }
            } else {
                dest->value.array_val.data = NULL;
                dest->value.array_val.size = 0;
                dest->value.array_val.capacity = 0;
            }
            break;
            
        case ZVAL_OBJECT:
        case ZVAL_CLOSURE:
        case ZVAL_RESOURCE:
            // TODO: Implement proper copying for these types
            dest->type = ZVAL_NULL;
            break;
    }
}

// Zval comparison
bool microphp_zval_equals(const zval_t *a, const zval_t *b) {
    if (!a || !b) return false;
    if (a->type != b->type) return false;
    
    switch (a->type) {
        case ZVAL_NULL:
            return true;
            
        case ZVAL_BOOL:
            return a->value.bool_val == b->value.bool_val;
            
        case ZVAL_INT:
            return a->value.int_val == b->value.int_val;
            
        case ZVAL_FLOAT:
            return a->value.float_val == b->value.float_val;
            
        case ZVAL_STRING:
            if (a->value.string_val.len != b->value.string_val.len) return false;
            if (a->value.string_val.str == b->value.string_val.str) return true;
            if (!a->value.string_val.str || !b->value.string_val.str) return false;
            return memcmp(a->value.string_val.str, b->value.string_val.str, a->value.string_val.len) == 0;
            
        case ZVAL_ARRAY:
            if (a->value.array_val.size != b->value.array_val.size) return false;
            for (size_t i = 0; i < a->value.array_val.size; i++) {
                if (!microphp_zval_equals(&a->value.array_val.data[i], &b->value.array_val.data[i])) {
                    return false;
                }
            }
            return true;
            
        case ZVAL_OBJECT:
        case ZVAL_CLOSURE:
        case ZVAL_RESOURCE:
            // TODO: Implement proper comparison for these types
            return false;
    }
    
    return false;
}

// Array operations
int microphp_array_push(zval_t *array, const zval_t *value) {
    if (!array || array->type != ZVAL_ARRAY || !value) return -1;
    
    // Grow array if needed
    if (array->value.array_val.size >= array->value.array_val.capacity) {
        size_t new_capacity = array->value.array_val.capacity == 0 ? 8 : array->value.array_val.capacity * 2;
        zval_t *new_data = realloc(array->value.array_val.data, new_capacity * sizeof(zval_t));
        if (!new_data) return -1;
        
        array->value.array_val.data = new_data;
        array->value.array_val.capacity = new_capacity;
        
        // Initialize new elements to null
        for (size_t i = array->value.array_val.size; i < new_capacity; i++) {
            array->value.array_val.data[i] = microphp_zval_null();
        }
    }
    
    // Add value
    microphp_zval_copy(&array->value.array_val.data[array->value.array_val.size], value);
    array->value.array_val.size++;
    
    return 0;
}

int microphp_array_get(const zval_t *array, size_t index, zval_t *result) {
    if (!array || array->type != ZVAL_ARRAY || !result) return -1;
    if (index >= array->value.array_val.size) return -1;
    
    microphp_zval_copy(result, &array->value.array_val.data[index]);
    return 0;
}

int microphp_array_set(zval_t *array, size_t index, const zval_t *value) {
    if (!array || array->type != ZVAL_ARRAY || !value) return -1;
    if (index >= array->value.array_val.size) return -1;
    
    microphp_zval_destroy(&array->value.array_val.data[index]);
    microphp_zval_copy(&array->value.array_val.data[index], value);
    return 0;
}

size_t microphp_array_size(const zval_t *array) {
    if (!array || array->type != ZVAL_ARRAY) return 0;
    return array->value.array_val.size;
}

// String operations
zval_t microphp_string_concat(const zval_t *a, const zval_t *b) {
    if (!a || !b) return microphp_zval_null();
    
    // Convert to strings if needed
    const char *a_str = NULL;
    size_t a_len = 0;
    const char *b_str = NULL;
    size_t b_len = 0;
    
    if (a->type == ZVAL_STRING) {
        a_str = a->value.string_val.str;
        a_len = a->value.string_val.len;
    } else if (a->type == ZVAL_INT) {
        // TODO: Convert int to string
        a_str = "";
        a_len = 0;
    } else if (a->type == ZVAL_FLOAT) {
        // TODO: Convert float to string
        a_str = "";
        a_len = 0;
    }
    
    if (b->type == ZVAL_STRING) {
        b_str = b->value.string_val.str;
        b_len = b->value.string_val.len;
    } else if (b->type == ZVAL_INT) {
        // TODO: Convert int to string
        b_str = "";
        b_len = 0;
    } else if (b->type == ZVAL_FLOAT) {
        // TODO: Convert float to string
        b_str = "";
        b_len = 0;
    }
    
    // Concatenate strings
    size_t total_len = a_len + b_len;
    char *result_str = malloc(total_len + 1);
    if (!result_str) return microphp_zval_null();
    
    if (a_str) memcpy(result_str, a_str, a_len);
    if (b_str) memcpy(result_str + a_len, b_str, b_len);
    result_str[total_len] = '\0';
    
    zval_t result = microphp_zval_string(result_str, total_len);
    free(result_str);
    
    return result;
}

int microphp_string_length(const zval_t *string) {
    if (!string || string->type != ZVAL_STRING) return -1;
    return (int)string->value.string_val.len;
}

// Built-in functions
zval_t microphp_builtin_print(const zval_t *args, size_t count) {
    for (size_t i = 0; i < count; i++) {
        const zval_t *arg = &args[i];
        switch (arg->type) {
            case ZVAL_NULL:
                printf("NULL");
                break;
            case ZVAL_BOOL:
                printf("%s", arg->value.bool_val ? "true" : "false");
                break;
            case ZVAL_INT:
                printf("%lld", arg->value.int_val);
                break;
            case ZVAL_FLOAT:
                printf("%f", arg->value.float_val);
                break;
            case ZVAL_STRING:
                printf("%.*s", (int)arg->value.string_val.len, arg->value.string_val.str);
                break;
            case ZVAL_ARRAY:
                printf("Array(%zu)", arg->value.array_val.size);
                break;
            default:
                printf("Unknown type");
                break;
        }
    }
    printf("\n");
    
    return microphp_zval_null();
}

zval_t microphp_builtin_sleep_ms(const zval_t *args, size_t count) {
    if (count < 1 || args[0].type != ZVAL_INT) {
        return microphp_zval_null();
    }
    
    // TODO: Implement actual sleep using platform-specific functions
    // For now, just return
    return microphp_zval_null();
}

zval_t microphp_builtin_millis(const zval_t *args, size_t count) {
    (void)args; // Unused
    (void)count; // Unused
    
    // TODO: Implement actual millisecond counter using platform-specific functions
    // For now, return 0
    return microphp_zval_int(0);
}
