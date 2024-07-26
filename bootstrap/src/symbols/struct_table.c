#include "struct_table.h"

#include "core/assert.h"
#include "core/memory.h"

#include "symbols/hash.h"

#include "namespace.h"

#include <string.h>

static FRX_NO_DISCARD b8 struct_symbol_equals(const StructSymbol* symbol,
                                              const char* name,
                                              const Namespace* namespace)
{
    return strcmp(symbol->name, name) == 0
        && namespace_equals(symbol->namespace, namespace);
}

void struct_table_init(StructTable* table)
{
    FRX_ASSERT(table != NULL);

    for(usize i = 0; i < FRX_STRUCT_TABLE_CAPACITY; ++i)
        table->entries[i] = NULL;
}

StructSymbol* struct_table_insert(StructTable* table, const char* name,
                                  const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_STRUCT_TABLE_CAPACITY;

    StructTableEntry* entry = memory_alloc(sizeof(StructTableEntry),
            FRX_MEMORY_CATEGORY_UNKNOWN);

    entry->next = table->entries[index];
    table->entries[index] = entry;

    StructSymbol* symbol = &entry->symbol;
    strcpy(symbol->name, name);
    symbol->namespace =
        namespace == NULL ? NULL : namespace_duplicate(namespace);

    list_init(&symbol->fields, FRX_MEMORY_CATEGORY_UNKNOWN);

    symbol->transpiled = FRX_FALSE;
    symbol->defined = FRX_FALSE;

    return symbol;
}

StructSymbol* struct_table_find(const StructTable* table, const char* name,
                                const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_STRUCT_TABLE_CAPACITY;

    StructTableEntry* entry = table->entries[index];
    while(entry != NULL
            && !struct_symbol_equals(&entry->symbol, name, namespace))
        entry = entry->next;

    return entry != NULL ? &entry->symbol : NULL;
}

StructSymbol* struct_table_find_or_insert(StructTable* table, const char* name,
                                          const Namespace* namespace)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    StructSymbol* symbol = struct_table_find(table, name, namespace);

    if(symbol == NULL)
        symbol = struct_table_insert(table, name, namespace);

    return symbol;
}

static void struct_symbol_delete(StructSymbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    namespace_delete(symbol->namespace);
}

static void struct_table_entry_delete(StructTableEntry* entry)
{
    if(entry == NULL)
        return;

    struct_table_entry_delete(entry->next);
    struct_symbol_delete(&entry->symbol);

    memory_free(entry);
}

void struct_table_delete(StructTable* table)
{
    FRX_ASSERT(table != NULL);

    for(usize i = 0; i < FRX_STRUCT_TABLE_CAPACITY; ++i)
        struct_table_entry_delete(table->entries[i]);
}
