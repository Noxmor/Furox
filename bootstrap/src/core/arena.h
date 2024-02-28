#ifndef FRX_ARENA_H
#define FRX_ARENA_H

#include "core/types.h"

typedef struct Arena
{
    void* buffer;
    usize pos;
    usize size;
} Arena;

void arena_init(Arena* arena, usize size);

void* arena_alloc(Arena* arena, usize size);

void arena_reset(Arena* arena);

void arena_free(Arena* arena);

#endif
