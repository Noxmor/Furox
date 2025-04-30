#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

void unary_expr_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    UnaryExpr* unary_expr = &ast->unary_expr;

    ast_resolve(unary_expr->operand, parser);
}

void unary_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(ctx != NULL);

    UnaryExpr* unary_expr = &ast->unary_expr;

    ast_sema(unary_expr->operand, ctx);
}

void unary_expr_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(ctx != NULL);

    UnaryExpr* unary_expr = &ast->unary_expr;

    fprintf(ctx->source, "(%s(", token_type_to_str(unary_expr->type));
    ast_codegen(unary_expr->operand, ctx);
    fprintf(ctx->source, "))");
}
