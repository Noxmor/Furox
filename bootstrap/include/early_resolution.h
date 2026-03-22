#ifndef FRX_EARLY_RESOLUTION_H
#define FRX_EARLY_RESOLUTION_H

#include "resolution.h"

void ast_resolve_early(AST* ast, ResolutionContext* ctx);

void translation_unit_resolve_early(AST* ast, ResolutionContext* ctx);

void use_tree_resolve_early(AST* ast, ResolutionContext* ctx);

void use_stmt_resolve_early(AST* ast, ResolutionContext* ctx);

void type_alias_resolve_early(AST* ast, ResolutionContext* ctx);

void enum_def_resolve_early(AST* ast, ResolutionContext* ctx);

void struct_def_resolve_early(AST* ast, ResolutionContext* ctx);

void trait_resolve_early(AST* ast, ResolutionContext* ctx);

void impl_block_resolve_early(AST* ast, ResolutionContext* ctx);

void func_param_resolve_early(AST* ast, ResolutionContext* ctx);

void func_decl_resolve_early(AST* ast, ResolutionContext* ctx);

#endif
