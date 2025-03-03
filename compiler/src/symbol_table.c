#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "log.h"
#include "hash.h"
#include "compiler.h"

#ifndef FRX_SYMBOL_INTERN_TABLE_CAPACITY
#define FRX_SYMBOL_INTERN_TABLE_CAPACITY 1024
#endif

static SymbolID next_free_symbol_id;

typedef struct SymbolInternEntry
{
    SymbolID id;
    const char* name;
    struct SymbolInternEntry* next;
} SymbolInternEntry;

static SymbolInternEntry* symbol_intern_entry_create(const char* name, SymbolInternEntry* next)
{
    SymbolInternEntry* entry = compiler_alloc(sizeof(SymbolInternEntry));

    entry->id = next_free_symbol_id++;
    entry->name = name;
    entry->next = next;

    return entry;
}

typedef struct SymbolInternTable
{
    SymbolInternEntry* entries[FRX_SYMBOL_INTERN_TABLE_CAPACITY];
} SymbolInternTable;

static SymbolInternTable intern_table;

SymbolID symbol_intern(const char* name)
{
    FRX_ASSERT(name != NULL);

    u64 index = hash_fnv1a(name) % FRX_SYMBOL_INTERN_TABLE_CAPACITY;
    SymbolInternEntry* entry = intern_table.entries[index];

    while (entry != NULL)
    {
        if (strcmp(entry->name, name) == 0)
        {
            return entry->id;
        }

        entry = entry->next;
    }

    SymbolInternEntry* new_entry = symbol_intern_entry_create(name, intern_table.entries[index]);
    intern_table.entries[index] = new_entry;

    return new_entry->id;
}

static Symbol* symbol_create(SymbolType type, void* data)
{
    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(data != NULL);

    Symbol* symbol = compiler_alloc(sizeof(Symbol));

    symbol->type = type;
    symbol->data = data;

    return symbol;
}

void symbol_table_init(SymbolTable* table)
{
    FRX_ASSERT(table != NULL);

    memset(table, 0, sizeof(SymbolTable));
}

Symbol* symbol_table_insert(SymbolTable* table, SymbolID id, SymbolType type, void* data)
{
    if (symbol_table_lookup(table, id) != NULL)
    {
        return NULL;
    }

    Symbol* symbol = symbol_create(type, data);
    symbol_table_insert_symbol(table, id, symbol);

    return symbol;
}

void symbol_table_insert_symbol(SymbolTable* table, SymbolID id, Symbol* symbol)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(symbol != NULL);

    if (symbol_table_lookup(table, id) != NULL)
    {
        return;
    }

    u64 index = id % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    new_entry->id = id;
    new_entry->symbol = symbol;
    new_entry->next = entry;

    table->entries[index] = new_entry;
}

Symbol* symbol_table_lookup(SymbolTable* table, SymbolID id)
{
    FRX_ASSERT(table != NULL);

    u64 index = id % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            return entry->symbol;
        }

        entry = entry->next;
    }

    return NULL;
}
