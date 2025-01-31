#ifndef FRX_STRING_TABLE_CAPACITY
#define FRX_STRING_TABLE_CAPACITY 1024
#endif

#include <string.h>
#include <stdlib.h>

#include "assert.h"
#include "hash.h"

typedef struct StringTableEntry
{
    char* str;
    struct StringTableEntry* next;
} StringTableEntry;

static StringTableEntry* string_table_entry_create(const char* str, StringTableEntry* next)
{
    FRX_ASSERT(str != NULL);

    StringTableEntry* entry = malloc(sizeof(StringTableEntry));
    entry->str = strdup(str);
    entry->next = next;

    return entry;
}

static void string_table_entry_destroy(StringTableEntry* entry)
{
    if (entry != NULL)
    {
        free(entry->str);
        string_table_entry_destroy(entry->next);
        free(entry);
    }
}

typedef struct StringTable
{
    StringTableEntry* entries[FRX_STRING_TABLE_CAPACITY];
} StringTable;

static StringTable table;

const char* string_table_intern(const char* str)
{
    FRX_ASSERT(str != NULL);

    u64 index = hash_djb2(str) % FRX_STRING_TABLE_CAPACITY;
    StringTableEntry* entry = table.entries[index];

    while(entry != NULL)
    {
        if(strcmp(entry->str, str) == 0)
        {
            return entry->str;
        }

        entry = entry->next;
    }

    StringTableEntry* new_entry = string_table_entry_create(str, entry);
    table.entries[index] = new_entry;

    return new_entry->str;
}

void string_table_shutdown(void)
{
    for (usize i = 0; i < FRX_STRING_TABLE_CAPACITY; ++i)
    {
        StringTableEntry* entry = table.entries[i];
        string_table_entry_destroy(entry);
    }
}
