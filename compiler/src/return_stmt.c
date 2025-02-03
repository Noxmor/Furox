#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static ReturnStmt* return_stmt_create(Expr* value)
{
    ReturnStmt* return_stmt = compiler_alloc(sizeof(ReturnStmt));

    return_stmt->value = value;

    return return_stmt;
}

ReturnStmt* return_stmt_parse(Parser* parser)
{
    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_RETURN))
    {
        return NULL;
    }

    Expr* value = NULL;

    if (!parser_match(parser, FRX_TOKEN_TYPE_SEMI))
    {
        value = expr_parse(parser);
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_SEMI))
    {
        return NULL;
    }

    return return_stmt_create(value);
}

void return_stmt_sema(ReturnStmt* return_stmt)
{
    FRX_ASSERT(return_stmt != NULL);

    if (return_stmt->value != NULL)
    {
        expr_sema(return_stmt->value);
    }
}

void return_stmt_codegen(ReturnStmt* return_stmt)
{
    FRX_ASSERT(return_stmt != NULL);

    codegen_write("return");

    if (return_stmt->value != NULL)
    {
        codegen_write(" ");
        expr_codegen(return_stmt->value);
    }

    codegen_write(";\n");
}

