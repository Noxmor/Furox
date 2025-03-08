#include "symbol_registry.h"
#include "assert.h"

void symbol_registry_init(SymbolRegistry* symbol_registry)
{
    FRX_ASSERT(symbol_registry != NULL);

    list_init(&symbol_registry->registry);
}

SymbolID symbol_registry_add(SymbolRegistry* symbol_registry, const char* name)
{
    FRX_ASSERT(symbol_registry != NULL);

    SymbolID id = list_size(&symbol_registry->registry);
    list_add(&symbol_registry->registry, (void*)name);

    return id;
}

void* symbol_registry_get(SymbolRegistry* symbol_registry, SymbolID id)
{
    FRX_ASSERT(symbol_registry != NULL);

    return list_get(&symbol_registry->registry, id);
}

void symbol_registry_destroy(SymbolRegistry* symbol_registry)
{
    FRX_ASSERT(symbol_registry != NULL);

    list_destroy(&symbol_registry->registry);
}
