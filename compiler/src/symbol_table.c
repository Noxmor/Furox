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

void symbol_table_insert(SymbolTable* table, SymbolType type, const char* name, SymbolID id)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(name != NULL);

    FRX_ASSERT(id != FRX_SYMBOL_ID_INVALID);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    new_entry->id = id;
    new_entry->type = type;
    new_entry->name = name;
    new_entry->next = entry;

    table->entries[index] = new_entry;
}

SymbolID symbol_table_lookup(SymbolTable* table, SymbolType type, const char* name)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(name != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        if (entry->type == type && entry->name == name)
        {
            return entry->id;
        }

        entry = entry->next;
    }

    return FRX_SYMBOL_ID_INVALID;
}
