#ifndef TYPES_H
#define TYPES_H

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_INT8,
    TYPE_INT32,
    TYPE_UINT,
    TYPE_UINT8,
    TYPE_UINT32,
    TYPE_UNKNOWN
} Type;

static inline const char* type_to_string(Type t) {
    switch (t) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_INT8: return "int8";
        case TYPE_INT32: return "int32";
        case TYPE_UINT: return "uint";
        case TYPE_UINT8: return "uint8";
        case TYPE_UINT32: return "uint32";
        default: return "unknown";
    }
}

#endif
