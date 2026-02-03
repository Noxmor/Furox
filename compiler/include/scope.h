#ifndef FRX_SCOPE_H
#define FRX_SCOPE_H

#include "symbol_table.h"

typedef struct Scope
{
    struct Scope* parent;
    SymbolTable symbols;
} Scope;

Scope* scope_create_global(void);

Scope* scope_create(Scope* parent);

Symbol* scope_insert_symbol(Scope* scope, SymbolVisibility visibility,
                            SymbolType type, const char* name, void* data);

Symbol* scope_lookup_symbol(Scope* scope, const char* name);

#endif
