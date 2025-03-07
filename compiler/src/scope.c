#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static Scope* scope_create(b8 error)
{
    Scope* scope = compiler_alloc(sizeof(Scope));

    scope->error = error;
    list_init(&scope->stmts);

    return scope;
}

static void scope_add_stmt(Scope* scope, Stmt* stmt)
{
    FRX_ASSERT(scope != NULL);
    FRX_ASSERT(stmt != NULL);

    list_add(&scope->stmts, stmt);
}

Scope* scope_from_stmt(Stmt* stmt)
{
    Scope* scope = scope_create(FRX_FALSE);
    scope_add_stmt(scope, stmt);

    return scope;
}

Scope* scope_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    Scope* scope = scope_create(error);
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        Stmt* stmt = stmt_parse(parser);
        scope_add_stmt(scope, stmt);
    }

    scope->error |= parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    return scope;
}

void scope_resolve(Parser* parser, Scope* scope)
{
    FRX_ASSERT(scope != NULL);

    if (scope->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        Stmt* stmt = list_get(&scope->stmts, i);
        stmt_resolve(parser, stmt);
    }
}

void scope_sema(Scope* scope)
{
    FRX_ASSERT(scope != NULL);

    if (scope->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        Stmt* stmt = list_get(&scope->stmts, i);
        stmt_sema(stmt);
    }
}

void scope_codegen(Scope* scope)
{
    FRX_ASSERT(scope != NULL);
    FRX_ASSERT(!scope->error);

    codegen_write("{\n");

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        Stmt* stmt = list_get(&scope->stmts, i);
        stmt_codegen(stmt);
    }

    codegen_write("}\n");
}
