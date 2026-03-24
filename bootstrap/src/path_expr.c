#include "assert.h"
#include "ast.h"
#include "resolution.h"
#include "type_system.h"

void path_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTPathExpr* path_expr = &ast->path_expr;

    path_resolve(path_expr->path, ctx);

    path_expr->resolved_type = symbol_infer_type(path_expr->path->path.symbol);
}
