#include "assert.h"
#include "compiler.h"
#include "scope.h"
#include "symbol_table.h"

Scope* scope_create_global(void)
{
    Scope* scope = compiler_alloc(sizeof(Scope));

    scope->parent = NULL;
    symbol_table_init(&scope->symbols);

    return scope;
}

Scope* scope_create(Scope* parent)
{
    FRX_ASSERT(parent != NULL);

    Scope* scope = compiler_alloc(sizeof(Scope));

    scope->parent = parent;
    symbol_table_init(&scope->symbols);

    return scope;
}

Symbol* scope_insert_symbol(Scope* scope, SymbolVisibility visibility,
                            SymbolType type, const char* name, void* data)
{
    Symbol* symbol = scope_lookup_symbol(scope, name);

    if (symbol != NULL)
    {
        return symbol;
    }

    return symbol_table_insert(&scope->symbols, visibility, type, name, data);
}

Symbol* scope_lookup_symbol(Scope* scope, const char* name)
{
    Symbol* symbol = NULL;

    while (symbol == NULL && scope != NULL)
    {
        symbol = symbol_table_lookup(&scope->symbols, name);
        scope = scope->parent;
    }

    return symbol;
}
