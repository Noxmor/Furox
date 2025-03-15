#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static ExprStmt* expr_stmt_create(b8 error, Expr* expr)
{
    ExprStmt* expr_stmt = compiler_alloc_ast(sizeof(ExprStmt));

    expr_stmt->error = error;
    expr_stmt->expr = expr;

    return expr_stmt;
}

ExprStmt* expr_stmt_parse(Parser* parser)
{
    b8 error = FRX_FALSE;
    Expr* expr = expr_parse(parser);

    error |= parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return expr_stmt_create(error, expr);
}

void expr_stmt_resolve(Parser* parser, ExprStmt* expr_stmt)
{
    FRX_ASSERT(expr_stmt != NULL);

    if (expr_stmt->error)
    {
        return;
    }

    expr_resolve(parser, expr_stmt->expr);
}

void expr_stmt_sema(ExprStmt* expr_stmt)
{
    FRX_ASSERT(expr_stmt != NULL);

    if (expr_stmt->error)
    {
        return;
    }

    expr_sema(expr_stmt->expr);
}
