#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static void scope_init(Scope* scope)
{
    list_init(&scope->stmts);
}

static void scope_add_stmt(Scope* scope, AST* stmt)
{
    FRX_ASSERT(scope != NULL);
    FRX_ASSERT(stmt != NULL);

    list_add(&scope->stmts, stmt);
}

AST* scope_from_stmt(AST* stmt)
{
    AST* ast = ast_create(FRX_AST_TYPE_SCOPE);
    Scope* scope = &ast->scope;

    scope_init(scope);
    scope_add_stmt(scope, stmt);

    return ast;
}

AST* scope_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_SCOPE);
    Scope* scope = &ast->scope;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    scope_init(scope);
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* stmt = stmt_parse(parser);
        scope_add_stmt(scope, stmt);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void scope_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SCOPE);

    Scope* scope = &ast->scope;

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        AST* stmt = list_get(&scope->stmts, i);
        ast_resolve(stmt, parser);
    }
}

void scope_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SCOPE);

    FRX_ASSERT(ctx != NULL);

    Scope* scope = &ast->scope;

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        AST* stmt = list_get(&scope->stmts, i);
        ast_sema(stmt, ctx);
    }
}

void scope_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SCOPE);

    FRX_ASSERT(ctx != NULL);

    Scope* scope = &ast->scope;

    fprintf(ctx->source, "{\n");

    for (usize i = 0; i < list_size(&scope->stmts); ++i)
    {
        AST* stmt = list_get(&scope->stmts, i);
        ast_codegen(stmt, ctx);
    }

    fprintf(ctx->source, "}\n");
}
