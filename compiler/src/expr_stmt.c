#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static ExprStmt* expr_stmt_create(Expr* expr)
{
    ExprStmt* expr_stmt = compiler_alloc(sizeof(ExprStmt));

    expr_stmt->expr = expr;

    return expr_stmt;
}

ExprStmt* expr_stmt_parse(Parser* parser)
{
    Expr* expr = expr_parse(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_SEMI))
    {
        return NULL;
    }

    return expr_stmt_create(expr);
}

void expr_stmt_sema(ExprStmt* expr_stmt)
{
    FRX_ASSERT(expr_stmt != NULL);

    expr_sema(expr_stmt->expr);
}

void expr_stmt_codegen(ExprStmt* expr_stmt)
{
    FRX_ASSERT(expr_stmt != NULL);

    expr_codegen(expr_stmt->expr);
    codegen_write(";\n");
}
