#ifndef FRX_ARENA_H
#define FRX_ARENA_H

#include "types.h"

typedef struct Arena
{
    void* start;
    void* current;
    struct Arena* next;

} Arena;

Arena* arena_create(void);

void* arena_alloc(Arena* arena, usize size);

void arena_reset(Arena* arena);

void arena_destroy(Arena* arena);

#endif
