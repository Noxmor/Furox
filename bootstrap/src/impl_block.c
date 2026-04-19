#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "early_resolution.h"
#include "late_resolution.h"
#include "resolution.h"
#include "sema.h"
#include "token.h"
#include "type_system.h"

static void impl_block_init(ASTImplBlock* impl_block, AST* generic_params,
                            AST* trait_path, AST* type_path)
{
    impl_block->generic_params = generic_params;
    impl_block->trait_path = trait_path;
    impl_block->type_path = type_path;
    list_init(&impl_block->methods);
}

AST* impl_block_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_IMPL_BLOCK);
    ASTImplBlock* impl_block = &ast->impl_block;

    attributes_table_insert_scope(ast->id, parser_push_scope(parser));

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL);

    AST* generic_params = NULL;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_LT)
    {
        generic_params = generic_params_parse(parser);
    }

    AST* trait_path = NULL;
    AST* type_path = NULL;

    type_path = path_parse(parser, FRX_PATH_STYLE_TYPE);

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_FOR)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_FOR);

        trait_path = type_path;
        type_path = path_parse(parser, FRX_PATH_STYLE_TYPE);
    }

    impl_block_init(impl_block, generic_params, trait_path, type_path);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        SymbolVisibility visibility = parse_visibility(parser);
        AST* func_decl = func_decl_parse(parser, visibility);
        list_add(&impl_block->methods, func_decl);
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_pop_scope(parser);

    return ast;
}

void impl_block_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    ASTImplBlock* impl_block = &ast->impl_block;

    resolution_context_push_scope(ctx, attributes_table_lookup_scope(ast->id));
    ctx->current_impl_block = ast;

    if (impl_block->trait_path != NULL)
    {
        path_resolve(impl_block->trait_path, ctx);
    }

    if (impl_block->type_path != NULL)
    {
        path_resolve(impl_block->type_path, ctx);
        const Type* type = symbol_infer_type(impl_block->type_path->path.symbol);

        for (usize i = 0; i < list_size(&impl_block->methods); ++i)
        {
            AST* func_decl = list_get(&impl_block->methods, i);
            func_decl_resolve_early(func_decl, ctx);

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
            func_decl_resolve_early(func_decl, ctx);
        }
    }

    resolution_context_pop_scope(ctx);
    ctx->current_impl_block = NULL;
}

void impl_block_resolve_late(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    ASTImplBlock* impl_block = &ast->impl_block;

    resolution_context_push_scope(ctx, attributes_table_lookup_scope(ast->id));
    ctx->current_impl_block = ast;

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_decl = list_get(&impl_block->methods, i);
        func_decl_resolve_late(func_decl, ctx);
    }

    resolution_context_pop_scope(ctx);
    ctx->current_impl_block = NULL;
}

void impl_block_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IMPL_BLOCK);

    FRX_ASSERT(ctx != NULL);

    ASTImplBlock* impl_block = &ast->impl_block;
    ctx->current_impl_block = impl_block;

    if (impl_block->trait_path != NULL)
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
