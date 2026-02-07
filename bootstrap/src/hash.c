#include "hash.h"

#include "assert.h"

u64 hash_djb2(const char* str)
{
    FRX_ASSERT(str != NULL);

    u64 hash = 5381;
    char c;

    while((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

u64 hash_fnv1a(const char* str)
{
    FRX_ASSERT(str != NULL);

    u64 hash = 0xCBF29CE484222325;

    while (*str)
    {
        hash ^= (u8)(*str++);
        hash *= 0x100000001B3;
    }

    return hash;
}
