#include "memory.h"

#include <stdlib.h>
#include <string.h>

#include "core/assert.h"

#ifdef FRX_TRACE_MEMORY

#define FRX_MEMORY_TABLE_SIZE 1024

typedef struct AllocInfo
{
    const void* memory;
    usize size;
    MemoryCategory category;
    struct AllocInfo* next;
} AllocInfo;

typedef struct MemoryCategoryInfo
{
    usize total_allocated;
    usize total_freed;
    usize total_reallocated;

    usize total_allocations;
    usize total_frees;
    usize total_reallocations;
} MemoryCategoryInfo;

typedef struct MemoryTracer
{
    MemoryCategoryInfo global_info;

    MemoryCategoryInfo category_info[FRX_MEMORY_CATEGORY_COUNT];

    AllocInfo memory_table[FRX_MEMORY_TABLE_SIZE];
} MemoryTracer;

static MemoryTracer memory_tracer;

static void memory_tracer_trace_alloc(usize size, MemoryCategory category)
{
    memory_tracer.global_info.total_allocated += size;
    ++memory_tracer.global_info.total_allocations;

    MemoryCategoryInfo* category_info = &memory_tracer.category_info[category];
    category_info->total_allocated += size;
    ++category_info->total_allocations;
}

static void memory_tracer_trace_free(usize size, MemoryCategory category)
{
    memory_tracer.global_info.total_freed += size;
    ++memory_tracer.global_info.total_frees;

    MemoryCategoryInfo* category_info = &memory_tracer.category_info[category];
    category_info->total_freed += size;
    ++category_info->total_frees;
}

static void memory_tracer_trace_realloc(usize size, MemoryCategory category)
{
    memory_tracer.global_info.total_reallocated += size;
    ++memory_tracer.global_info.total_reallocations;

    MemoryCategoryInfo* category_info = &memory_tracer.category_info[category];
    category_info->total_reallocated += size;
    ++category_info->total_reallocations;
}
static const char* memory_category_to_str[] =
{
    "Unknown",
    "String",
    "Ast"
};

static void print_alloc_info(const AllocInfo* alloc_info, usize level)
{
    for(usize i = 0; i < level; ++i)
        printf("  ");

    printf("Node: Memory: %zu, Size: %zu, Category: %s\n", (usize)alloc_info->memory, alloc_info->size, memory_category_to_str[alloc_info->category]);

    if(alloc_info->next != NULL)
        print_alloc_info(alloc_info->next, level + 1);
}

void memory_table_print(void)
{
    for(usize i = 0; i < FRX_MEMORY_TABLE_SIZE; ++i)
    {
        const AllocInfo* alloc_info = &memory_tracer.memory_table[i];

        if(alloc_info->memory == 0)
            continue;

        print_alloc_info(alloc_info, 0);
    }
}

static u64 memory_table_hash(const void* memory)
{
    u64 hash = (u64)memory % FRX_MEMORY_TABLE_SIZE;

    return hash;
}

static void memory_table_insert(const void* memory, usize size, MemoryCategory category)
{
    u64 hash = memory_table_hash(memory);

    AllocInfo* alloc_info = &memory_tracer.memory_table[hash];

    while(alloc_info->memory != 0)
    {
        if(alloc_info->next == NULL)
            alloc_info->next = calloc(1, sizeof(AllocInfo));

        alloc_info = alloc_info->next;
    }

    alloc_info->memory = memory;
    alloc_info->size = size;
    alloc_info->category = category;
}

static AllocInfo* memory_table_find(const void* memory)
{
    u64 hash = memory_table_hash(memory);

    AllocInfo* alloc_info = &memory_tracer.memory_table[hash];

    while(alloc_info != NULL && alloc_info->memory != memory)
        alloc_info = alloc_info->next;

    return alloc_info;
}

static void memory_table_remove(const void* memory)
{
    u64 hash = memory_table_hash(memory);

    AllocInfo* alloc_info = &memory_tracer.memory_table[hash];
    
    if(alloc_info->memory == memory)
    {
        if(alloc_info->next != NULL)
        {
            AllocInfo* alloc_info_to_free = alloc_info->next;
            memcpy(alloc_info, alloc_info->next, sizeof(AllocInfo));
            free(alloc_info_to_free);
        }
        else
            memset(alloc_info, 0, sizeof(AllocInfo));

        return;
    }

    AllocInfo* alloc_info_prev = alloc_info;
    alloc_info = alloc_info->next;

    while(alloc_info != NULL && alloc_info->memory != memory)
    {
        alloc_info_prev = alloc_info;
        alloc_info = alloc_info->next;
    }

    if(alloc_info != NULL)
    {
        alloc_info_prev->next = alloc_info->next;
        free(alloc_info);
    }
}

void* memory_alloc(usize size, MemoryCategory category)
{
    FRX_ASSERT(size > 0);

    void* memory = malloc(size);

    memory_tracer_trace_alloc(size, category);

    memory_table_insert(memory, size, category);

    return memory;
}

void* memory_realloc(void* memory, usize size, MemoryCategory category)
{
    FRX_ASSERT(size > 0);

    if(memory == NULL)
        return memory_alloc(size, category);

    AllocInfo* alloc_info = memory_table_find(memory);

    FRX_ASSERT(alloc_info != NULL);

    void* new_memory = realloc(memory, size);

    memory_tracer_trace_free(alloc_info->size, alloc_info->category);
    memory_tracer_trace_alloc(size, category);
    memory_tracer_trace_realloc(size, category);

    memory_table_remove(memory);
    memory_table_insert(new_memory, size, category);

    return new_memory;
}

void memory_free(void* memory)
{
    AllocInfo* alloc_info = memory_table_find(memory);

    FRX_ASSERT(alloc_info != NULL);

    memory_tracer_trace_free(alloc_info->size, alloc_info->category);
    
    free(memory);

    memory_table_remove(memory);
}

void memory_print(void)
{
    printf("Total allocated: %zu\n", memory_tracer.global_info.total_allocated);
    printf("Total freed: %zu\n", memory_tracer.global_info.total_freed);
    printf("Total reallocated: %zu\n", memory_tracer.global_info.total_reallocated);

    printf("Total allocations: %zu\n", memory_tracer.global_info.total_allocations);
    printf("Total frees: %zu\n", memory_tracer.global_info.total_frees);
    printf("Total reallocations: %zu\n", memory_tracer.global_info.total_reallocations);

    printf("\n");

    for(usize i = 0; i < FRX_MEMORY_CATEGORY_COUNT; ++i)
    {
        MemoryCategoryInfo* info = &memory_tracer.category_info[i];
        printf("%s: Total allocated: %zu, Total freed: %zu, Total reallocated: %zu\n", memory_category_to_str[i], info->total_allocated, info->total_freed, info->total_reallocated);
        printf("%s: Total allocations: %zu, Total frees: %zu, Total reallocations: %zu\n", memory_category_to_str[i], info->total_allocations, info->total_frees, info->total_reallocations);
        printf("\n");
    }
}

#else

void* memory_alloc(usize size, MemoryCategory category)
{
    (void)category;

    return malloc(size);
}

void* memory_realloc(void* memory, usize size, MemoryCategory category)
{
    (void)category;
    
    return realloc(memory, size);
}

void memory_free(void* memory)
{
    free(memory);
}

void memory_print(void)
{
    
}

void memory_table_print(void)
{

}

#endif
