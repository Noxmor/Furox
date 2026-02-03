#ifndef FRX_SOURCE_FILE_H
#define FRX_SOURCE_FILE_H

#include "types.h"
#include "list.h"
#include "ast.h"
#include "module.h"
#include "diagnostics.h"
#include "scope.h"

typedef usize SourceFileID;

typedef struct SourceFile
{
    const char* path;
    SourceFileID id;
    char* data;
    usize data_len;
    Module* module;
    AST* ast;
    List diagnostics;
    Scope* global_scope;
} SourceFile;

b8 source_file_load_from_disk(SourceFile* source_file, const char* filepath);

void source_file_add_diagnostic(SourceFile* source_file, Diagnostic* d);

Symbol* source_file_insert_symbol(SourceFile* source_file, SymbolVisibility visibility,
                                  SymbolType type, const char* name, void* data);

Symbol* source_file_lookup_symbol(SourceFile* source_file, const char* name);

const char* source_file_data(const SourceFile* source_file);

usize source_file_data_len(const SourceFile* source_file);

#endif
