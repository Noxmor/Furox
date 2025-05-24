#ifndef FRX_SOURCE_FILE_H
#define FRX_SOURCE_FILE_H

#include "types.h"

typedef u32 SourceFileID;

typedef struct SourceFile
{
    SourceFileID id;
    const char* path;
    char* data;
    usize data_len;
} SourceFile;

b8 source_file_load_from_disk(SourceFile* source_file, const char* filepath);

#endif
