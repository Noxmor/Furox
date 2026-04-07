#include "assert.h"
#include "parser.h"
#include "resolution.h"

AST* self_expr_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_SELF_EXPR);
    ASTSelfExpr* self_expr = &ast->self_expr;

    ast->span = parser_current_span(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_SELF_LOWER);

    self_expr->resolved_type = NULL;

    return ast;
}

void self_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SELF_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTSelfExpr* self_expr = &ast->self_expr;

    self_expr->resolved_type = ctx->current_func_decl->func_decl.receiver_type;
}
