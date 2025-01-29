#include "list.h"

#include <stdlib.h>

#include "assert.h"

void list_init(List* list)
{
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

void list_add(List* list, void* item)
{
    if (list->size >= list->capacity)
    {
        list->capacity = list->capacity ? list->capacity * 2 : 1;
        list->items = realloc(list->items, sizeof(void*) * list->capacity);
    }

    list->items[list->size++] = item;
}

usize list_size(const List* list)
{
    return list->size;
}

void* list_get(const List* list, usize index)
{
    FRX_ASSERT(index < list->size);

    return list->items[index];
}
