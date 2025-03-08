#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "compiler.h"

void symbol_table_init(SymbolTable* table)
{
    FRX_ASSERT(table != NULL);

    memset(table, 0, sizeof(SymbolTable));
}

void symbol_table_insert(SymbolTable* table, Parser* origin,
                         SymbolVisibility visibility, SymbolType type,
                         const char* name, void* data)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(origin != NULL);

    FRX_ASSERT(visibility < FRX_SYMBOL_VISIBILITY_COUNT);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(data != NULL);

    if (symbol_table_lookup(table, origin, name) != NULL)
    {
        return;
    }

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    new_entry->origin = origin;
    new_entry->visibility = visibility;
    new_entry->name = name;
    new_entry->symbol.type = type;
    new_entry->symbol.data = data;
    new_entry->next = entry;

    table->entries[index] = new_entry;
}

Symbol* symbol_table_lookup(SymbolTable* table, Parser* origin, const char* name)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(origin != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        if (entry->name == name && (entry->origin == origin || entry->visibility == FRX_SYMBOL_VISIBILITY_PUBLIC))
        {
            return &entry->symbol;
        }

        entry = entry->next;
    }

    return NULL;
}
