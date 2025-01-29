#include "arena.h"

#include <stdlib.h>

#include "assert.h"

#define FRX_ARENA_CAPACITY 1024 * 1024

Arena* arena_create(void)
{
    Arena* arena = malloc(sizeof(Arena));

    arena->start = malloc(FRX_ARENA_CAPACITY);
    arena->current = arena->start;
    arena->next = NULL;

    return arena;
}

void* arena_alloc(Arena* arena, usize size)
{
    FRX_ASSERT(size <= FRX_ARENA_CAPACITY);

    if (arena->current + size > arena->start + FRX_ARENA_CAPACITY)
    {
        if(arena->next != NULL)
        {
            return arena_alloc(arena->next, size);
        }

        Arena* new_arena = arena_create();
        arena->next = new_arena;
        arena = arena->next;
    }

    void* memory = arena->current;
    arena->current += size;

    return memory;
}

void arena_reset(Arena* arena)
{
    while (arena != NULL)
    {
        arena->current = arena->start;
        arena = arena->next;
    }
}

void arena_destroy(Arena* arena)
{
    while (arena)
    {
        Arena* next = arena->next;
        free(arena->start);
        free(arena);
        arena = next;
    }
}
