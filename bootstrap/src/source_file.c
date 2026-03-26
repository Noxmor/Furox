#include "source_file.h"

#include <stdlib.h>
#include <stdio.h>

#include "assert.h"
#include "compiler.h"

SourceFile* source_file_load_from_disk(const char* filepath, SourceOffset offset)
{
    FRX_ASSERT(filepath != NULL);

    FILE* f = fopen(filepath, "rb");
    if (f == NULL)
    {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    usize data_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = malloc(data_len + 1);
    usize read = fread(data, 1, data_len, f);

    fclose(f);

    if (read != data_len)
    {
        free(data);
        return NULL;
    }

    SourceFile* source_file = compiler_alloc(sizeof(SourceFile));

    source_file->path = filepath;
    source_file->data = data;
    source_file->data_len = data_len;
    source_file->offset = offset;

    source_file->data[source_file->data_len] = '\0';

    source_file->module = compiler_root_module();
    source_file->global_scope = scope_create_global();

    return source_file;
}

Symbol* source_file_insert_symbol(SourceFile* source_file, SymbolVisibility visibility,
                                  SymbolType type, const char* name, void* data)
{
    FRX_ASSERT(source_file != NULL);

    return scope_insert_symbol(source_file->global_scope, visibility, type, name, data);
}

Symbol* source_file_lookup_symbol(SourceFile* source_file, const char* name)
{
    FRX_ASSERT(source_file != NULL);

    return scope_lookup_symbol(source_file->global_scope, name);
}

const char* source_file_filepath(const SourceFile* source_file)
{
    FRX_ASSERT(source_file != NULL);

    return source_file->path;
}

const char* source_file_data(const SourceFile* source_file)
{
    FRX_ASSERT(source_file != NULL);

    return source_file->data;
}

usize source_file_data_len(const SourceFile* source_file)
{
    FRX_ASSERT(source_file != NULL);

    return source_file->data_len;
}

SourceOffset source_file_offset(const SourceFile* source_file)
{
    FRX_ASSERT(source_file != NULL);

    return source_file->offset;
}

void source_file_resolve_offset(const SourceFile* source_file, SourceOffset offset,
                              SourceLine* line, SourceColumn* column)
{
    FRX_ASSERT(source_file != NULL);

    FRX_ASSERT(offset >= source_file->offset);

    FRX_ASSERT(offset <= source_file->offset + source_file->data_len);

    FRX_ASSERT(line != NULL);

    FRX_ASSERT(column != NULL);

    *line = 0;
    *column = 0;

    SourceOffset pos = source_file->offset;
    const char* data = source_file->data;

    while (pos++ != offset)
    {
        if (*data++ == '\n')
        {
            *line += 1;
            *column = 0;
        }
        else
        {
            *column += 1;
        }
    }
}
