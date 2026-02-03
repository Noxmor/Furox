#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void impl_block_init(ASTImplBlock* impl_block, const char* type_name)
{
    FRX_ASSERT(type_name != NULL);

    impl_block->type_name = type_name;
    list_init(&impl_block->methods);
}

AST* impl_block_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_IMPL_BLOCK);
    ASTImplBlock* impl_block = &ast->impl_block;
    impl_block->scope = parser_push_scope(parser);

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL);

    const char* type_name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    impl_block_init(impl_block, type_name);
    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* func_decl = func_decl_parse(parser);
        list_add(&impl_block->methods, func_decl);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_pop_scope(parser);

    return ast;
}

void impl_block_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);


    ASTImplBlock* impl_block = &ast->impl_block;

    resolution_context_push_scope(ctx, impl_block->scope);

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_decl = list_get(&impl_block->methods, i);
        func_decl_resolve(func_decl, ctx);
    }

    resolution_context_pop_scope(ctx);
}

void impl_block_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    FRX_ASSERT(ctx != NULL);

    ASTImplBlock* impl_block = &ast->impl_block;

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_decl = list_get(&impl_block->methods, i);
        func_decl_sema(func_decl, ctx);
    }
}
