#include "arena.h"

#include "core/assert.h"
#include "core/memory.h"

void arena_init(Arena* arena, usize size)
{
    FRX_ASSERT(arena != NULL);

    FRX_ASSERT(size > 0);

    arena->size = size;
    arena->pos = 0;

    arena->buffer = memory_alloc(arena->size, FRX_MEMORY_CATEGORY_UNKNOWN);
}

void* arena_alloc(Arena* arena, usize size)
{
    FRX_ASSERT(arena != NULL);

    FRX_ASSERT(size > 0);

    FRX_ASSERT(arena->buffer != NULL);

    FRX_ASSERT(arena->pos + size <= arena->size);

    void* block = arena->buffer + arena->pos;

    arena->pos += size;

    return block;
}

void arena_reset(Arena* arena)
{
    FRX_ASSERT(arena != NULL);

    arena->pos = 0;
}

void arena_free(Arena* arena)
{
    FRX_ASSERT(arena != NULL);

    memory_free(arena->buffer);
}
