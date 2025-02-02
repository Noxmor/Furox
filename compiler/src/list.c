#include "list.h"

#include <stdlib.h>

#include "assert.h"

void list_init(List* list)
{
    FRX_ASSERT(list != NULL);

    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

void list_add(List* list, void* item)
{
    FRX_ASSERT(list != NULL);

    if (list->size >= list->capacity)
    {
        list->capacity = list->capacity ? list->capacity * 2 : 1;
        list->items = realloc(list->items, sizeof(void*) * list->capacity);
    }

    list->items[list->size++] = item;
}

usize list_size(const List* list)
{
    FRX_ASSERT(list != NULL);

    return list->size;
}

b8 list_empty(const List* list)
{
    return list_size(list) == 0;
}

void* list_get(const List* list, usize index)
{
    FRX_ASSERT(list != NULL);
    FRX_ASSERT(index < list->size);

    return list->items[index];
}
