#ifndef FRX_COMPILER_H
#define FRX_COMPILER_H

#include "types.h"
#include "list.h"
#include "module.h"
#include "ast.h"

int compiler_run(int argc, char** argv);

void* compiler_alloc(usize size);

void* compiler_alloc_ast(usize size);

void* compiler_alloc_mir(usize size);

Module* compiler_root_module(void);

Module* compiler_find_module_by_path_segments(const List* path_segments);

void compiler_register_expr(AST* expr);

#endif
