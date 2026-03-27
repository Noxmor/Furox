#include "assert.h"
#include "source_map.h"
#include "source_file.h"
#include "list.h"

typedef struct SourceMap
{
    List source_files;
    SourceOffset current_offset;
} SourceMap;

static SourceMap source_map;

void source_map_init(void)
{
    list_init(&source_map.source_files);
}

void source_map_add_source_file(const char* filepath)
{
    FRX_ASSERT(filepath != NULL);

    SourceFile* source_file = source_file_load_from_disk(filepath, source_map.current_offset);
    if (source_file != NULL)
    {
        source_map.current_offset += source_file_buffer(source_file)->offset + 1;
        list_add(&source_map.source_files, source_file);
    }
}

SourceFile* source_map_lookup_source_file(SourceOffset offset)
{
    for (usize i = 0; i < list_size(&source_map.source_files); ++i)
    {
        SourceFile* source_file = list_get(&source_map.source_files, i);
        const SourceBuffer* buffer = source_file_buffer(source_file);

        if (buffer->offset <= offset && offset <= buffer->offset + buffer->len)
        {
            return source_file;
        }
    }

    FRX_ASSERT(FRX_FALSE);

    return NULL;
}

void source_map_resolve_offset(SourceOffset offset, SourceLine* line, SourceColumn* column)
{
    FRX_ASSERT(line != NULL);

    FRX_ASSERT(column != NULL);

    SourceFile* source_file = source_map_lookup_source_file(offset);

    source_file_resolve_offset(source_file, offset, line, column);
}

const char* source_span_filepath(SourceSpan span)
{
    SourceFile* source_file = source_map_lookup_source_file(span.lo);

    return source_file_filepath(source_file);
}

List* source_map_get_source_files(void)
{
    return &source_map.source_files;
}
