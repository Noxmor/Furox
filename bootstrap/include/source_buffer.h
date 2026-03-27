#ifndef FRX_SOURCE_BUFFER_H
#define FRX_SOURCE_BUFFER_H

#include "source_span.h"

typedef struct SourceBuffer
{
    const char* src;
    usize len;
    SourceOffset offset;
} SourceBuffer;

void source_buffer_init(SourceBuffer* buffer, const char* src, usize len, SourceOffset offset);

void source_buffer_resolve_offset(const SourceBuffer* buffer, SourceOffset offset,
                              SourceLine* line, SourceColumn* column);

#endif
