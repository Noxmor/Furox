#ifndef FRX_LATE_RESOLUTION_H
#define FRX_LATE_RESOLUTION_H

#include "resolution.h"

void ast_resolve_late(AST* ast, ResolutionContext* ctx);

void translation_unit_resolve_late(AST* ast, ResolutionContext* ctx);

void impl_block_resolve_late(AST* ast, ResolutionContext* ctx);

void func_decl_resolve_late(AST* ast, ResolutionContext* ctx);

#endif
