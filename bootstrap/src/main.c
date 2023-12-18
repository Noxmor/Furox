#include <stdio.h>

#include "core/memory.h"

int main(void)
{
    memory_print();

    void* mem0 = memory_alloc(512, FRX_MEMORY_CATEGORY_UNKNOWN);

    void* mem1 = memory_alloc(256, FRX_MEMORY_CATEGORY_STRING);
    
    void* mem2 = memory_alloc(32, FRX_MEMORY_CATEGORY_UNKNOWN);

    memory_print();

    memory_free(mem0);
    memory_free(mem1);
    
    memory_print();

    mem2 = memory_realloc(mem2, 64, FRX_MEMORY_CATEGORY_STRING);

    memory_print();

    return 0;
}
