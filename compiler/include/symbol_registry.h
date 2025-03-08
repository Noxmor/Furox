#ifndef FRX_SYMBOL_REGISTRY_H
#define FRX_SYMBOL_REGISTRY_H

#include "list.h"

#define FRX_SYMBOL_ID_INVALID 0xFFFFFFFFFFFFFFFF

typedef usize SymbolID;

typedef struct SymbolRegistry
{
    List registry;
} SymbolRegistry;

void symbol_registry_init(SymbolRegistry* symbol_registry);

SymbolID symbol_registry_add(SymbolRegistry* symbol_registry, const char* name);

void* symbol_registry_get(SymbolRegistry* symbol_registry, SymbolID id);

void symbol_registry_destroy(SymbolRegistry* symbol_registry);

#endif
