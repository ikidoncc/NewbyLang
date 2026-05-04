#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable *symtab_new() {
    SymbolTable *tab = malloc(sizeof(SymbolTable));
    tab->symbols = NULL;
    tab->count = 0;
    return tab;
}

void symtab_add(SymbolTable *tab, char *name, int offset, Type type) {
    tab->count++;
    tab->symbols = realloc(tab->symbols, sizeof(Symbol) * tab->count);
    tab->symbols[tab->count - 1].name = strdup(name);
    tab->symbols[tab->count - 1].stack_offset = offset;
    tab->symbols[tab->count - 1].type = type;
}

Symbol *symtab_lookup(SymbolTable *tab, char *name) {
    for (int i = 0; i < tab->count; i++) {
        if (strcmp(tab->symbols[i].name, name) == 0) {
            return &tab->symbols[i];
        }
    }
    return NULL;
}
