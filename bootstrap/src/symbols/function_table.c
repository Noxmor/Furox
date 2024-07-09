#include "function_table.h"

#include <string.h>

#include "core/assert.h"

#include "symbols/hash.h"

static FRX_NO_DISCARD b8 function_symbol_equals(const FunctionSymbol* symbol,
                                                const char* name,
                                                const Namespace* namespace)
{
    return strcmp(symbol->name, name) == 0
        && namespace_equals(symbol->namespace, namespace);
}

void function_table_init(FunctionTable* table)
{
    FRX_ASSERT(table != NULL);

    for(usize i = 0; i < FRX_FUNCTION_TABLE_CAPACITY; ++i)
        table->entries[i] = NULL;
}

FunctionSymbol* function_table_insert(FunctionTable* table, const char* name,
                                      const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_FUNCTION_TABLE_CAPACITY;

    FunctionTableEntry* entry = memory_alloc(sizeof(FunctionTableEntry),
            FRX_MEMORY_CATEGORY_UNKNOWN);

    entry->next = table->entries[index];
    table->entries[index] = entry;

    FunctionSymbol* symbol = &entry->symbol;
    strcpy(symbol->name, name);
    symbol->namespace =
        namespace == NULL ? NULL : namespace_duplicate(namespace);

    symbol->defined = FRX_FALSE;

    return symbol;
}

FunctionSymbol* function_table_find(const FunctionTable* table,
                                    const char* name,
                                    const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_FUNCTION_TABLE_CAPACITY;

    FunctionTableEntry* entry = table->entries[index];
    while(entry != NULL
            && !function_symbol_equals(&entry->symbol, name, namespace))
        entry = entry->next;

    return entry != NULL ? &entry->symbol : NULL;
}

FunctionSymbol* function_table_find_or_insert(FunctionTable* table,
                                              const char* name,
                                              const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    FunctionSymbol* symbol = function_table_find(table, name, namespace);

    if(symbol == NULL)
        symbol = function_table_insert(table, name, namespace);

    return symbol;
}

static void function_symbol_delete(FunctionSymbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    namespace_delete(symbol->namespace);
}

static void function_table_entry_delete(FunctionTableEntry* entry)
{
    if(entry == NULL)
        return;

    function_table_entry_delete(entry->next);
    function_symbol_delete(&entry->symbol);

    memory_free(entry);
}

void function_table_delete(FunctionTable* table)
{
    FRX_ASSERT(table != NULL)

    for(usize i = 0; i < FRX_FUNCTION_TABLE_CAPACITY; ++i)
        function_table_entry_delete(table->entries[i]);
}
