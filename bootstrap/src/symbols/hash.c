#include "hash.h"

#include "core/assert.h"

u64 hash_djb2(const char *str)
{
    FRX_ASSERT(str != NULL);

    u64 hash = 5381;

    char c;
    while((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}
