#include "list.h"

#include "core/assert.h"

void list_init(List* list, MemoryCategory category)
{
    FRX_ASSERT(list != NULL);

    list->items = NULL;
    list->category = category;

    list->size = 0;
    list->capacity = 0;
}

void list_push(List* list, void* item)
{
    FRX_ASSERT(list != NULL);

    if(list->size >= list->capacity)
    {
        list->capacity = list->capacity ? list->capacity * 2 : 1;
        list->items = memory_realloc(list->items, list->capacity * sizeof(void*), list->category);
    }

    list->items[list->size++] = item;
}

void* list_get(const List* list, usize index)
{
    FRX_ASSERT(list != NULL);

    FRX_ASSERT(index < list->size);

    return list->items[index];
}

usize list_size(const List* list)
{
    FRX_ASSERT(list != NULL);

    return list->size;
}

void list_free(List* list)
{
    FRX_ASSERT(list != NULL);

    memory_free(list->items);
}
