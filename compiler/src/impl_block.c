#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void impl_block_init(ImplBlock* impl_block, const char* type_name)
{
    FRX_ASSERT(type_name != NULL);

    impl_block->type_name = type_name;
    list_init(&impl_block->methods);
}

AST* impl_block_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_IMPL_BLOCK);
    ImplBlock* impl_block = &ast->impl_block;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL);

    const char* type_name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    impl_block_init(impl_block, type_name);
    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* func_def = func_def_parse(parser);
        list_add(&impl_block->methods, func_def);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    return ast;
}

void impl_block_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    ImplBlock* impl_block = &ast->impl_block;

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_def = list_get(&impl_block->methods, i);
        func_def_resolve(func_def, parser);
    }
}

void impl_block_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    FRX_ASSERT(ctx != NULL);

    ImplBlock* impl_block = &ast->impl_block;

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_def = list_get(&impl_block->methods, i);
        func_def_sema(func_def, ctx);
    }
}
