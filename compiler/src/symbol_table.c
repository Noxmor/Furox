#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
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

void symbol_table_init(SymbolTable* table)
{
    FRX_ASSERT(table != NULL);

    memset(table, 0, sizeof(SymbolTable));
}

void symbol_table_insert(SymbolTable* table, Parser* origin,
                         SymbolVisibility visibility, SymbolID id,
                         SymbolType type, void* data)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(origin != NULL);

    FRX_ASSERT(visibility < FRX_SYMBOL_VISIBILITY_COUNT);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    FRX_ASSERT(data != NULL);

    if (symbol_table_lookup(table, origin, id) != NULL)
    {
        return;
    }

    u64 index = id % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    SymbolTableEntry* new_entry = compiler_alloc(sizeof(SymbolTableEntry));
    new_entry->origin = origin;
    new_entry->visibility = visibility;
    new_entry->id = id;
    new_entry->symbol.type = type;
    new_entry->symbol.data = data;
    new_entry->next = entry;

    table->entries[index] = new_entry;
}

Symbol* symbol_table_lookup(SymbolTable* table, Parser* origin, SymbolID id)
{
    FRX_ASSERT(table != NULL);

    FRX_ASSERT(origin != NULL);

    u64 index = id % FRX_SYMBOL_TABLE_CAPACITY;
    SymbolTableEntry* entry = table->entries[index];

    while (entry != NULL)
    {
        //TODO: Check the parser's use statements for a possible match
        if (entry->id == id && entry->origin == origin)
        {
            return &entry->symbol;
        }

        entry = entry->next;
    }

    return NULL;
}
