#ifndef FRX_LIST_H
#define FRX_LIST_H

#include "types.h"

typedef struct List
{
    void** items;
    usize size;
    usize capacity;
} List;

void list_init(List* list);

void list_add(List* list, void* item);

usize list_size(const List* list);

b8 list_empty(const List* list);

void* list_get(const List* list, usize index);

#endif
