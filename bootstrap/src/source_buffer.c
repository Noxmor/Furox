#include "assert.h"
#include "source_buffer.h"

void source_buffer_init(SourceBuffer* buffer, const char* src, usize len, SourceOffset offset)
{
    FRX_ASSERT(buffer != NULL);

    FRX_ASSERT(src != NULL);

    buffer->src = src;
    buffer->len = len;
    buffer->offset = offset;
}

void source_buffer_resolve_offset(const SourceBuffer* buffer, SourceOffset offset,
                                  SourceLine* line, SourceColumn* column)
{
    FRX_ASSERT(buffer != NULL);

    FRX_ASSERT(offset >= buffer->offset);

    FRX_ASSERT(offset <= buffer->offset + buffer->len);

    FRX_ASSERT(line != NULL);

    FRX_ASSERT(column != NULL);

    *line = 1;
    *column = 1;

    SourceOffset pos = buffer->offset;
    const char* data = buffer->src;

    while (pos++ != offset)
    {
        if (*data++ == '\n')
        {
            *line += 1;
            *column = 1;
        }
        else
        {
            *column += 1;
        }
    }
}
