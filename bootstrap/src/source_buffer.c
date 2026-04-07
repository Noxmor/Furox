#include "assert.h"
#include "source_buffer.h"

#include <string.h>

void source_buffer_init(SourceBuffer* buffer, const char* src, usize len)
{
    FRX_ASSERT(buffer != NULL);

    FRX_ASSERT(src != NULL);

    FRX_ASSERT(strlen(src) == len);

    buffer->src = src;
    buffer->len = len;
}
