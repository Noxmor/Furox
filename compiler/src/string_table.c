#ifndef FRX_STRING_TABLE_CAPACITY
#define FRX_STRING_TABLE_CAPACITY 1024
#endif

#include <string.h>
#include <stdlib.h>

#include "assert.h"
#include "hash.h"

typedef struct StringTableEntry
{
    const char* str;
    struct StringTableEntry* next;
} StringTableEntry;

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

    StringTableEntry* new_entry = malloc(sizeof(StringTableEntry));
    new_entry->str = strdup(str);
    new_entry->next = entry;

    table.entries[index] = new_entry;

    return new_entry->str;
}
