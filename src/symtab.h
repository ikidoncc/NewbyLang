#ifndef SYMTAB_H
#define SYMTAB_H

#include "types.h"

typedef struct {
    char *name;
    int stack_offset;
    Type type;
    int is_array;
    int array_size;
} Symbol;

typedef struct SymbolTable {
    Symbol *symbols;
    int count;
    struct SymbolTable *parent;
} SymbolTable;

SymbolTable *symtab_new(SymbolTable *parent);
void symtab_add(SymbolTable *tab, char *name, int offset, Type type, int is_array, int array_size);
Symbol *symtab_lookup(SymbolTable *tab, char *name);

#endif
