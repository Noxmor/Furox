#include "variable_table.h"

#include "core/core.h"
#include "core/assert.h"
#include "core/memory.h"

#include "symbols/hash.h"

#include "namespace.h"

#include <string.h>

static FRX_NO_DISCARD b8 variable_symbol_equals(const VariableSymbol* symbol,
                                                const char* name,
                                                const Namespace* namespace)
{
    FRX_ASSERT(symbol != NULL);

    FRX_ASSERT(name != NULL);

    return strcmp(symbol->name, name) == 0
        && namespace_equals(symbol->namespace, namespace);
}

void variable_table_init(VariableTable* table)
{
    FRX_ASSERT(table != NULL);

    for(usize i = 0; i < FRX_VARIABLE_TABLE_CAPACITY; ++i)
        table->entries[i] = NULL;
}

VariableSymbol* variable_table_insert(VariableTable* table, const char* name,
                                      const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_VARIABLE_TABLE_CAPACITY;

    VariableTableEntry* entry = memory_alloc(sizeof(VariableTableEntry),
            FRX_MEMORY_CATEGORY_UNKNOWN);

    entry->next = table->entries[index];
    table->entries[index] = entry;

    VariableSymbol* symbol = &entry->symbol;
    strcpy(symbol->name, name);
    symbol->namespace =
        namespace == NULL ? NULL : namespace_duplicate(namespace);

    return symbol;
}

VariableSymbol* variable_table_find(const VariableTable* table,
                                    const char* name,
                                    const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_VARIABLE_TABLE_CAPACITY;

    VariableTableEntry* entry = table->entries[index];
    while(entry != NULL
            && !variable_symbol_equals(&entry->symbol, name, namespace))
        entry = entry->next;

    return entry != NULL ? &entry->symbol : NULL;
}

static void variable_symbol_delete(VariableSymbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    namespace_delete(symbol->namespace);
}

static void variable_table_entry_delete(VariableTableEntry* entry)
{
    if(entry == NULL)
        return;

    variable_table_entry_delete(entry->next);
    variable_symbol_delete(&entry->symbol);

    memory_free(entry);
}

void variable_table_delete(VariableTable *table)
{
    FRX_ASSERT(table != NULL);

    for(usize i = 0; i < FRX_VARIABLE_TABLE_CAPACITY; ++i)
    {
        VariableTableEntry* entry = table->entries[i];

        if(entry != NULL)
            variable_table_entry_delete(entry);
    }
}
