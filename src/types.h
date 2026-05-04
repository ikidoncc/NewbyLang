#ifndef TYPES_H
#define TYPES_H

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_UNKNOWN
} Type;

static inline const char* type_to_string(Type t) {
    switch (t) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        default: return "unknown";
    }
}

#endif
