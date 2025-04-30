#ifndef FRX_RESOLUTION_H
#define FRX_RESOLUTION_H

#include "parser.h"

void ast_resolve(AST* ast, Parser* parser);

void translation_unit_resolve(AST* ast, Parser* parser);

void use_stmt_resolve(AST* ast, Parser* parser);

void type_specifier_resolve(AST* ast, Parser* parser);

void struct_def_resolve(AST* ast, Parser* parser);

void enum_def_resolve(AST* ast, Parser* parser);

void trait_resolve(AST* ast, Parser* parser);

void impl_block_resolve(AST* ast, Parser* parser);

void func_params_resolve(AST* ast, Parser* parser);

void func_decl_resolve(AST* ast, Parser* parser);

void func_def_resolve(AST* ast, Parser* parser);

void scope_resolve(AST* ast, Parser* parser);

void expr_stmt_resolve(AST* ast, Parser* parser);

void return_stmt_resolve(AST* ast, Parser* parser);

void let_stmt_resolve(AST* ast, Parser* parser);

void if_stmt_resolve(AST* ast, Parser* parser);

void unary_expr_resolve(AST* ast, Parser* parser);

void binary_expr_resolve(AST* ast, Parser* parser);

void path_expr_resolve(AST* ast, Parser* parser);

void call_expr_resolve(AST* ast, Parser* parser);

#endif
