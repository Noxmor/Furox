#ifndef FRX_TRANSPILER_H
#define FRX_TRANSPILER_H

#include "core/core.h"

#include "ast.h"

FRX_NO_DISCARD b8 transpile_ast(const AST* root, const char* src_filepath);

#endif
