#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void block_init(ASTBlock* block)
{
    list_init(&block->stmts);
}

static void block_add_stmt(ASTBlock* block, AST* stmt)
{
    FRX_ASSERT(block != NULL);
    FRX_ASSERT(stmt != NULL);

    list_add(&block->stmts, stmt);
}

AST* block_parse(Parser* parser)
{

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_BLOCK);
    ASTBlock* block = &ast->block;

    attributes_table_insert_scope(ast->id, parser_push_scope(parser));

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    block_init(block);
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* stmt = stmt_parse(parser);
        block_add_stmt(block, stmt);
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_pop_scope(parser);

    return ast;
}

void block_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BLOCK);

    ASTBlock* block = &ast->block;

    resolution_context_push_scope(ctx, attributes_table_lookup_scope(ast->id));

    for (usize i = 0; i < list_size(&block->stmts); ++i)
    {
        AST* stmt = list_get(&block->stmts, i);
        ast_resolve(stmt, ctx);
    }

    resolution_context_pop_scope(ctx);
}

void block_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BLOCK);

    FRX_ASSERT(ctx != NULL);

    ASTBlock* block = &ast->block;

    for (usize i = 0; i < list_size(&block->stmts); ++i)
    {
        AST* stmt = list_get(&block->stmts, i);
        ast_sema(stmt, ctx);
    }
}
