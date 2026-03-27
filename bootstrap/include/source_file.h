#ifndef FRX_SOURCE_FILE_H
#define FRX_SOURCE_FILE_H

#include "types.h"
#include "ast.h"
#include "module.h"
#include "scope.h"
#include "source_buffer.h"

typedef usize SourceFileID;

typedef struct SourceFile
{
    const char* path;
    SourceFileID id;
    SourceBuffer buffer;
    Module* module;
    AST* ast;
    Scope* global_scope;
} SourceFile;

SourceFile* source_file_load_from_disk(const char* filepath, SourceOffset offset);

Symbol* source_file_insert_symbol(SourceFile* source_file, SymbolVisibility visibility,
                                  SymbolType type, const char* name, void* data);

Symbol* source_file_lookup_symbol(SourceFile* source_file, const char* name);

const char* source_file_filepath(const SourceFile* source_file);

const SourceBuffer* source_file_buffer(const SourceFile* source_file);

void source_file_resolve_offset(const SourceFile* source_file, SourceOffset offset,
                              SourceLine* line, SourceColumn* column);

#endif
