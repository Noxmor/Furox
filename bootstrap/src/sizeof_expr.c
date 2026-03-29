#include "assert.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

AST* sizeof_expr_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_SIZEOF_EXPR);
    ASTSizeofExpr* sizeof_expr = &ast->sizeof_expr;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_SIZEOF);

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);
    sizeof_expr->expr = expr_parse(parser);

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return ast;
}

void sizeof_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SIZEOF_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTSizeofExpr* sizeof_expr = &ast->sizeof_expr;

    ast_resolve(sizeof_expr->expr, ctx);
}

void sizeof_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SIZEOF_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTSizeofExpr* sizeof_expr = &ast->sizeof_expr;

    ast_sema(sizeof_expr->expr, ctx);
}
