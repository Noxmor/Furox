#ifndef FRX_SOURCE_BUFFER_H
#define FRX_SOURCE_BUFFER_H

#include "types.h"

typedef struct SourceBuffer
{
    const char* src;
    usize len;
} SourceBuffer;

void source_buffer_init(SourceBuffer* buffer, const char* src, usize len);

#endif
