#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "log.h"
#include "hash.h"

#ifndef FRX_SYMBOL_INTERN_TABLE_CAPACITY
#define FRX_SYMBOL_INTERN_TABLE_CAPACITY 1024
#endif

static SymbolID next_free_symbol_id;

typedef struct SymbolInternEntry
{
    Symbol symbol;
    struct SymbolInternEntry* next;
} SymbolInternEntry;

static SymbolInternEntry* symbol_intern_entry_create(const char* name, SymbolInternEntry* next)
{
    SymbolInternEntry* entry = malloc(sizeof(SymbolInternEntry));

    entry->symbol.name = name;
    entry->symbol.id = next_free_symbol_id++;
    entry->next = next;

    return entry;
}

static void symbol_intern_entry_destroy(SymbolInternEntry* entry)
{
    if (entry != NULL)
    {
        symbol_intern_entry_destroy(entry->next);
        free(entry);
    }
}

typedef struct SymbolInternTable
{
    SymbolInternEntry* entries[FRX_SYMBOL_INTERN_TABLE_CAPACITY];
} SymbolInternTable;

static SymbolInternTable intern_table;

Symbol* symbol_intern(const char* name)
{
    FRX_ASSERT(name != NULL);

    u64 index = hash_fnv1a(name) % FRX_SYMBOL_INTERN_TABLE_CAPACITY;
    SymbolInternEntry* entry = intern_table.entries[index];

    while (entry != NULL)
    {
        if (strcmp(entry->symbol.name, name) == 0)
        {
            return &entry->symbol;
        }

        entry = entry->next;
    }

    SymbolInternEntry* new_entry = symbol_intern_entry_create(name, intern_table.entries[index]);
    intern_table.entries[index] = new_entry;

    return &new_entry->symbol;
}

void symbol_table_shutdown(void)
{
    FRX_LOG_INFO("Shutting down symbol table...");

    for (usize i = 0; i < FRX_SYMBOL_INTERN_TABLE_CAPACITY; ++i)
    {
        SymbolInternEntry* entry = intern_table.entries[i];
        symbol_intern_entry_destroy(entry);
    }
}
