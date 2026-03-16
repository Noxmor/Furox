#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "token.h"
#include "type_system.h"

static void impl_block_init(ASTImplBlock* impl_block, AST* generic_params,
                            AST* trait_path_expr, AST* type_path_expr, TokenType primitive)
{
    impl_block->generic_params = generic_params;
    impl_block->trait_path_expr = trait_path_expr;
    impl_block->type_path_expr = type_path_expr;
    impl_block->primitive = primitive;
    list_init(&impl_block->methods);
}

AST* impl_block_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_IMPL_BLOCK);
    ASTImplBlock* impl_block = &ast->impl_block;
    impl_block->scope = parser_push_scope(parser);

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL);

    AST* generic_params = NULL;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_LT)
    {
        generic_params = generic_params_parse(parser);
    }

    AST* trait_path_expr = NULL;
    AST* type_path_expr = NULL;
    TokenType primitive = FRX_TOKEN_TYPE_EOF;

    if (token_type_is_primitive(parser_current_type(parser)))
    {
        primitive = parser_current_type(parser);
        parser_eat(parser, primitive);
    }
    else
    {
        type_path_expr = path_expr_parse(parser);
    }

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_FOR)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_FOR);

        trait_path_expr = type_path_expr;

        if (token_type_is_primitive(parser_current_type(parser)))
        {
            type_path_expr = NULL;
            primitive = parser_current_type(parser);
            parser_eat(parser, primitive);
        }
        else
        {
            type_path_expr = path_expr_parse(parser);
        }
    }

    impl_block_init(impl_block, generic_params, trait_path_expr, type_path_expr,
                    primitive);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        SymbolVisibility visibility = parse_visibility(parser);
        AST* func_decl = func_decl_parse(parser, visibility);
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

    if (impl_block->trait_path_expr != NULL)
    {
        path_expr_resolve(impl_block->trait_path_expr, ctx);
    }

    if (impl_block->type_path_expr != NULL)
    {
        path_expr_resolve(impl_block->type_path_expr, ctx);
        const Type* type = expr_infer_type(impl_block->type_path_expr);

        for (usize i = 0; i < list_size(&impl_block->methods); ++i)
        {
            AST* func_decl = list_get(&impl_block->methods, i);
            func_decl_resolve(func_decl, ctx);

            Symbol* symbol = resolution_context_lookup_symbol(ctx, func_decl->func_decl.name);
            FRX_ASSERT(symbol != NULL);
            symbol->associated_type = type;

            type_register_method(type, symbol);
        }
    }
    else
    {
        for (usize i = 0; i < list_size(&impl_block->methods); ++i)
        {
            AST* func_decl = list_get(&impl_block->methods, i);
            func_decl_resolve(func_decl, ctx);
        }
    }

    resolution_context_pop_scope(ctx);
}

void impl_block_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    FRX_ASSERT(ctx != NULL);

    ASTImplBlock* impl_block = &ast->impl_block;
    ctx->current_impl_block = impl_block;

    if (impl_block->trait_path_expr != NULL)
    {
        // TODO: Check that the function signatures of this impl block
        // match the function signatures of the specified trait.
    }

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_decl = list_get(&impl_block->methods, i);
        func_decl_sema(func_decl, ctx);
    }

    ctx->current_impl_block = NULL;
}
