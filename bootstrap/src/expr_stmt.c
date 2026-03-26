#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

AST* expr_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_EXPR_STMT);
    ast->span.lo = parser_current_span(parser).lo;
    ASTExprStmt* expr_stmt = &ast->expr_stmt;

    expr_stmt->expr = expr_parse(parser);

    ast->span.hi = parser_current_span(parser).hi;

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return ast;
}

void expr_stmt_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTExprStmt* expr_stmt = &ast->expr_stmt;

    ast_resolve(expr_stmt->expr, ctx);
}

void expr_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTExprStmt* expr_stmt = &ast->expr_stmt;

    ast_sema(expr_stmt->expr, ctx);
}
