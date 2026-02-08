#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "scope.h"
#include "sema.h"
#include "type_system.h"

static void impl_block_init(ASTImplBlock* impl_block, TokenType primitive,
                            AST* path_expr)
{
    impl_block->primitive = primitive;
    impl_block->path_expr = path_expr;
    list_init(&impl_block->methods);
}

AST* impl_block_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_IMPL_BLOCK);
    ASTImplBlock* impl_block = &ast->impl_block;
    impl_block->scope = parser_push_scope(parser);

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL);

    TokenType primitive = FRX_TOKEN_TYPE_EOF;
    AST* path_expr = NULL;

    if (token_type_is_primitive(parser_current_type(parser)))
    {
        primitive = parser_current_type(parser);
        parser_eat(parser, primitive);
    }
    else
    {
        path_expr = path_expr_parse(parser);
    }

    impl_block_init(impl_block, primitive, path_expr);
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

    if (impl_block->path_expr != NULL)
    {
        path_expr_resolve(impl_block->path_expr, ctx);
        const Type* type = expr_infer_type(impl_block->path_expr);

        for (usize i = 0; i < list_size(&impl_block->methods); ++i)
        {
            AST* func_decl = list_get(&impl_block->methods, i);
            type_register_method(type, scope_lookup_symbol(impl_block->scope, func_decl->func_decl.name));
        }
    }

    resolution_context_push_scope(ctx, impl_block->scope);

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        AST* func_decl = list_get(&impl_block->methods, i);
        func_decl_resolve(func_decl, ctx);

        Symbol* symbol = resolution_context_lookup_symbol(ctx, func_decl->func_decl.name);
        symbol->associated_type = expr_infer_type(impl_block->path_expr);
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
