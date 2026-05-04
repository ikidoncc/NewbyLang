#ifndef SYMTAB_H
#define SYMTAB_H

#include "types.h"

typedef struct {
    char *name;
    int stack_offset;
    Type type;
} Symbol;

typedef struct {
    Symbol *symbols;
    int count;
} SymbolTable;

SymbolTable *symtab_new();
void symtab_add(SymbolTable *tab, char *name, int offset, Type type);
Symbol *symtab_lookup(SymbolTable *tab, char *name);

#endif
