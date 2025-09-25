#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "compiler.h"

static void symbol_init(Symbol* symbol, SymbolVisibility visibility,
                        SymbolType type, void* data)
{
    FRX_ASSERT(symbol != NULL);

    symbol->visibility = visibility;
    symbol->type = type;
    symbol->data = data;
}

void symbol_table_init(SymbolTable* table, SymbolTable* parent)
{
    FRX_ASSERT(table != NULL);

    memset(table, 0, sizeof(SymbolTable));

    table->parent = parent;
}

Symbol* symbol_table_insert(SymbolTable* table, SymbolVisibility visibility,
                            SymbolType type, const char* name, void* data)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(name != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    symbol_init(&new_entry->symbol, visibility, type, data);
    new_entry->name = name;
    new_entry->next = entry;

    table->entries[index] = new_entry;

    return &new_entry->symbol;
}

Symbol* symbol_table_lookup(const SymbolTable* table, SymbolType type, const char* name)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(name != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        if (entry->symbol.type == type && entry->name == name)
        {
            return &entry->symbol;
        }

        entry = entry->next;
    }

    return NULL;
}

Symbol* symbol_table_lookup_type(const SymbolTable* table, const char* name)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        if (entry->name == name)
        {
            switch (entry->symbol.type)
            {
                case FRX_SYMBOL_TYPE_STRUCT:
                case FRX_SYMBOL_TYPE_UNION:
                case FRX_SYMBOL_TYPE_ENUM:
                {
                    return &entry->symbol;
                }
            }
        }

        entry = entry->next;
    }

    return NULL;
}
