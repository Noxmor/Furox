#include "source_file.h"

#include <stdlib.h>
#include <stdio.h>

b8 source_file_load_from_disk(SourceFile* source_file, const char* filepath)
{
    FRX_ASSERT(source_file != NULL);

    FRX_ASSERT(filepath != NULL);

    source_file->id = 0; // TODO: Determine id
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

    fread(source_file->data, 1, source_file->data_len, f);
    fclose(f);

    source_file->data[source_file->data_len] = '\0';

    return FRX_FALSE;
}
