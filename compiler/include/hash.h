#ifndef FRX_HASH_H
#define FRX_HASH_H

#include "types.h"

u64 hash_djb2(const char* str);

u64 hash_fnv1a(const char* str);

#endif
