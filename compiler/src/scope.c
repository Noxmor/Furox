#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static Scope* scope_create(void)
{
    Scope* scope = compiler_alloc(sizeof(Scope));

    list_init(&scope->stmts);

    return scope;
}

static void scope_add_stmt(Scope* scope, Stmt* stmt)
{
    FRX_ASSERT(scope != NULL);
    FRX_ASSERT(stmt != NULL);

    list_add(&scope->stmts, stmt);
}

Scope* scope_parse(Parser* parser)
{
    if (parser_eat(parser, FRX_TOKEN_TYPE_LBRACE))
    {
        return NULL;
    }

    Scope* scope = scope_create();
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        Stmt* stmt = stmt_parse(parser);
        if(stmt == NULL)
        {
            break;
        }

        scope_add_stmt(scope, stmt);
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        return NULL;
    }

    return scope;
}

void scope_sema(Scope* scope)
{
    FRX_ASSERT(scope != NULL);

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        Stmt* stmt = list_get(&scope->stmts, i);
        stmt_sema(stmt);
    }
}

void scope_codegen(Scope* scope)
{
    FRX_ASSERT(scope != NULL);

    codegen_write("{\n");

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        Stmt* stmt = list_get(&scope->stmts, i);
        stmt_codegen(stmt);
    }

    codegen_write("}\n");
}
