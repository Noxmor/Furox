#ifndef FRX_LIST_H
#define FRX_LIST_H

#include "core/types.h"
#include "core/memory.h"

typedef struct List
{
    void** items;
    MemoryCategory category;

    usize size;
    usize capacity;
} List;

void list_init(List* list, MemoryCategory category);

void list_push(List* list, void* item);

void* list_get(const List* list, usize index);

usize list_size(const List* list);

void list_free(List* list);

#endif
