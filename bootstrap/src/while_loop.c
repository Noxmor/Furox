#include "assert.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void while_loop_init(ASTWhileLoop* while_loop, AST* condition, AST* body)
{
    FRX_ASSERT(while_loop != NULL);

    FRX_ASSERT(condition != NULL);

    FRX_ASSERT(body != NULL);

    while_loop->condition = condition;
    while_loop->body = body;
}

AST* while_loop_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_WHILE_LOOP);
    ASTWhileLoop* while_loop = &ast->while_loop;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_WHILE);

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);
    AST* condition = expr_parse(parser);
    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    AST* body = block_parse(parser);

    ast->span.hi = body->span.hi;

    while_loop_init(while_loop, condition, body);

    return ast;
}

void while_loop_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_WHILE_LOOP);

    FRX_ASSERT(ctx != NULL);

    ASTWhileLoop* while_loop = &ast->while_loop;

    ast_resolve(while_loop->condition, ctx);
    block_resolve(while_loop->body, ctx);
}

void while_loop_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_WHILE_LOOP);

    FRX_ASSERT(ctx != NULL);

    ASTWhileLoop* while_loop = &ast->while_loop;

    ast_sema(while_loop->condition, ctx);
    block_sema(while_loop->body, ctx);
}
