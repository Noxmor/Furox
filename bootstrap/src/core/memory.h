#ifndef FRX_MEMORY_H
#define FRX_MEMORY_H

#include "core/types.h"

enum
{
    FRX_MEMORY_CATEGORY_UNKNOWN = 0,
    FRX_MEMORY_CATEGORY_STRING,

    FRX_MEMORY_CATEGORY_COUNT
};

typedef u8 MemoryCategory;

void* memory_alloc(usize size, MemoryCategory category);

void* memory_realloc(void* memory, usize size, MemoryCategory category);

void memory_free(void* memory);

void memory_print(void);

#endif
