#include "assert.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void loop_init(ASTLoop* loop, AST* body)
{
    FRX_ASSERT(loop != NULL);

    FRX_ASSERT(body != NULL);

    loop->body = body;
}

AST* loop_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_LOOP);
    ASTLoop* loop = &ast->loop;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_LOOP);

    AST* body = block_parse(parser);

    ast->span.hi = body->span.hi;

    loop_init(loop, body);

    return ast;
}

void loop_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LOOP);

    FRX_ASSERT(ctx != NULL);

    ASTLoop* loop = &ast->loop;

    block_resolve(loop->body, ctx);
}

void loop_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LOOP);

    FRX_ASSERT(ctx != NULL);

    ASTLoop* loop = &ast->loop;

    block_sema(loop->body, ctx);
}
