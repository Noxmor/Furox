#include "source_file.h"

#include <stdlib.h>
#include <stdio.h>

#include "assert.h"
#include "compiler.h"
#include "symbol_table.h"

b8 source_file_load_from_disk(SourceFile* source_file, const char* filepath)
{
    FRX_ASSERT(source_file != NULL);

    FRX_ASSERT(filepath != NULL);

    source_file->path = filepath;
    source_file->data = NULL;
    source_file->data_len = 0;

    FILE* f = fopen(source_file->path, "rb");
    if (f == NULL)
    {
        return FRX_TRUE;
    }

    fseek(f, 0, SEEK_END);
    source_file->data_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    source_file->data = malloc(source_file->data_len + 1);

    usize read = fread(source_file->data, 1, source_file->data_len, f);
    fclose(f);

    if (read != source_file->data_len)
    {
        return FRX_TRUE;
    }

    source_file->data[source_file->data_len] = '\0';

    source_file->module = compiler_root_module();
    list_init(&source_file->diagnostics);
    source_file->global_scope = scope_create_global();

    return FRX_FALSE;
}

void source_file_add_diagnostic(SourceFile* source_file, Diagnostic* d)
{
    FRX_ASSERT(source_file != NULL);

    FRX_ASSERT(d != NULL);

    list_add(&source_file->diagnostics, d);
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
