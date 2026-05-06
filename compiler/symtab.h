#ifndef SYMTAB_H
#define SYMTAB_H

#include "types.h"

typedef struct {
    char *name;
    int stack_offset;
    Type type;
    int is_array; // 0: var, 1: array, 2: func, 3: struct instance, 4: enum instance
    int array_size;
    char *struct_name;
} Symbol;

typedef struct SymbolTable {
    Symbol *symbols;
    int count;
    struct SymbolTable *parent;
} SymbolTable;

SymbolTable *symtab_new(SymbolTable *parent);
void symtab_add(SymbolTable *tab, char *name, int offset, Type type, int is_array, int array_size, const char *struct_name);
Symbol *symtab_lookup(SymbolTable *tab, char *name);

#endif
