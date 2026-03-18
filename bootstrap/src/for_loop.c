#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void for_loop_init(ASTForLoop* for_loop, AST* init, AST* condition,
                          AST* increment, AST* body)
{
    FRX_ASSERT(for_loop != NULL);

    for_loop->init = init;
    for_loop->condition = condition;
    for_loop->increment = increment;
    for_loop->body = body;
}

AST* for_loop_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_FOR_LOOP);
    ASTForLoop* for_loop = &ast->for_loop;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_FOR);
    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    parser_push_scope(parser);

    AST* init = NULL;
    if (parser_current_type(parser) != FRX_TOKEN_TYPE_SEMI)
    {
        if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_LET)
        {
            init = let_stmt_parse(parser);
        }
        else
        {
            init = expr_parse(parser);
            parser_eat(parser, FRX_TOKEN_TYPE_SEMI);
        }
    }
    else
    {
        parser_eat(parser, FRX_TOKEN_TYPE_SEMI);
    }

    AST* condition = expr_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    AST* increment = NULL;
    if (parser_current_type(parser) != FRX_TOKEN_TYPE_RPAREN)
    {
        increment = expr_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    AST* body = block_parse(parser);

    parser_pop_scope(parser);

    for_loop_init(for_loop, init, condition, increment, body);

    return ast;
}

void for_loop_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FOR_LOOP);

    FRX_ASSERT(ctx != NULL);

    ASTForLoop* for_loop = &ast->for_loop;

    if (for_loop->init != NULL)
    {
        ast_resolve(for_loop->init, ctx);
    }

    ast_resolve(for_loop->condition, ctx);

    if (for_loop->increment != NULL)
    {
        ast_resolve(for_loop->increment, ctx);
    }

    block_resolve(for_loop->body, ctx);
}

void for_loop_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FOR_LOOP);

    FRX_ASSERT(ctx != NULL);

    ASTForLoop* for_loop = &ast->for_loop;

    if (for_loop->init != NULL)
    {
        ast_sema(for_loop->init, ctx);
    }

    ast_sema(for_loop->condition, ctx);

    if (for_loop->increment != NULL)
    {
        ast_sema(for_loop->increment, ctx);
    }

    block_sema(for_loop->body, ctx);
}
