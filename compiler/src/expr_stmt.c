#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static ExprStmt* expr_stmt_create(Expr* expr)
{
    ExprStmt* expr_stmt = compiler_alloc_ast(sizeof(ExprStmt));

    expr_stmt->expr = expr;

    return expr_stmt;
}

ExprStmt* expr_stmt_parse(Parser* parser)
{
    Expr* expr = expr_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return expr_stmt_create(expr);
}

void expr_stmt_resolve(Parser* parser, ExprStmt* expr_stmt)
{
    FRX_ASSERT(expr_stmt != NULL);

    expr_resolve(parser, expr_stmt->expr);
}

void expr_stmt_sema(ExprStmt* expr_stmt)
{
    FRX_ASSERT(expr_stmt != NULL);

    expr_sema(expr_stmt->expr);
}
