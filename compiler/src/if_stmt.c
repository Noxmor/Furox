#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static IfStmt* if_stmt_create(b8 error, Expr* condition, Scope* if_block, Scope* else_block)
{
    IfStmt* if_stmt = compiler_alloc_ast(sizeof(IfStmt));

    if_stmt->error = error;
    if_stmt->condition = condition;
    if_stmt->if_block = if_block;
    if_stmt->else_block = else_block;

    return if_stmt;
}

IfStmt* if_stmt_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_IF);

    Expr* condition = expr_parse(parser);
    Scope* if_block = scope_parse(parser);
    Scope* else_block = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_ELSE))
    {
        error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_ELSE);

        if (parser_match(parser, FRX_TOKEN_TYPE_KW_IF))
        {
            else_block = scope_from_stmt(stmt_parse(parser));
        }
        else
        {
            else_block = scope_parse(parser);
        }
    }

    return if_stmt_create(error, condition, if_block, else_block);
}

void if_stmt_resolve(Parser* parser, IfStmt* if_stmt)
{
    FRX_ASSERT(if_stmt != NULL);

    if (if_stmt->error)
    {
        return;
    }

    if (if_stmt->condition != NULL)
    {
        expr_resolve(parser, if_stmt->condition);
    }

    if (if_stmt->if_block != NULL)
    {
        scope_resolve(parser, if_stmt->if_block);
    }

    if (if_stmt->else_block != NULL)
    {
        scope_resolve(parser, if_stmt->else_block);
    }
}

void if_stmt_sema(IfStmt* if_stmt)
{
    FRX_ASSERT(if_stmt != NULL);

    if (if_stmt->error)
    {
        return;
    }

    if (if_stmt->condition != NULL)
    {
        expr_sema(if_stmt->condition);
    }

    if (if_stmt->if_block != NULL)
    {
        scope_sema(if_stmt->if_block);
    }

    if (if_stmt->else_block != NULL)
    {
        scope_sema(if_stmt->else_block);
    }
}

void if_stmt_codegen(IfStmt* if_stmt)
{
    FRX_ASSERT(if_stmt != NULL);
    FRX_ASSERT(!if_stmt->error);

    codegen_write("if (");
    expr_codegen(if_stmt->condition);
    codegen_write(")\n");
    scope_codegen(if_stmt->if_block);

    if (if_stmt->else_block != NULL)
    {
        codegen_write("else\n");
        scope_codegen(if_stmt->else_block);
    }
}
