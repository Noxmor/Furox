#include "symbol_table.h"

#include <string.h>

#include "assert.h"
#include "compiler.h"


void symbol_table_init(SymbolTable* table)
{
    FRX_ASSERT(table != NULL);

    memset(table, 0, sizeof(SymbolTable));
}

Symbol* symbol_table_insert(SymbolTable* table, SymbolVisibility visibility,
                            SymbolType type, const char* name, void* data)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(name != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    if (entry != NULL)
    {
        return NULL;
    }

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    new_entry->symbol = symbol_create(name, visibility, type, data);
    new_entry->next = entry;

    table->entries[index] = new_entry;

    return new_entry->symbol;
}

Symbol* symbol_table_insert_symbol(SymbolTable* table, Symbol* symbol)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(symbol != NULL);

    u64 index = (usize)symbol->name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    if (entry != NULL)
    {
        return NULL;
    }

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    new_entry->symbol = symbol;
    new_entry->next = entry;

    table->entries[index] = new_entry;

    return new_entry->symbol;

}

Symbol* symbol_table_lookup(const SymbolTable* table, const char* name)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(name != NULL);

    u64 index = (usize)name % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        if (entry->symbol->name == name)
        {
            return entry->symbol;
        }

        entry = entry->next;
    }

    return NULL;
}
