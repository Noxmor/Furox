#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "early_resolution.h"
#include "late_resolution.h"
#include "sema.h"
#include "symbol.h"
#include "type_system.h"

static void func_decl_init(ASTFuncDecl* func_decl, const char* name, b8 external,
                           FuncReceiver receiver, b8 is_variadic, AST* generic_params,
                           AST* return_type, AST* body)
{
    FRX_ASSERT(name != NULL);

    func_decl->name = name;
    func_decl->external = external;
    func_decl->generic_params = generic_params;
    func_decl->receiver = receiver;
    func_decl->return_type = NULL;
    func_decl->is_variadic = is_variadic;
    func_decl->return_type = return_type;
    func_decl->body = body;
}

AST* func_decl_parse(Parser* parser, SymbolVisibility visibility)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_FUNC_DECL);
    ASTFuncDecl* func_decl = &ast->func_decl;
    list_init(&func_decl->params);

    attributes_table_insert_scope(ast->id, parser_push_scope(parser));

    ast->span.lo = parser_current_span(parser).lo;

    b8 external = FRX_FALSE;
    if (parser_match(parser, FRX_TOKEN_TYPE_KW_EXTERN))
    {
        external = FRX_TRUE;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN);
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_FN))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    const char* name = parse_ident(parser);
    if (name == NULL)
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    AST* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    FuncReceiver receiver = FRX_FUNC_RECEIVER_NONE;
    b8 is_variadic = FRX_FALSE;

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&func_decl->params))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        if (list_empty(&func_decl->params) && parser_current_type(parser) == FRX_TOKEN_TYPE_KW_SELF_LOWER)
        {
            parser_eat(parser, FRX_TOKEN_TYPE_KW_SELF_LOWER);

            if (parser_current_type(parser) == FRX_TOKEN_TYPE_STAR)
            {
                parser_eat(parser, FRX_TOKEN_TYPE_STAR);
                receiver = FRX_FUNC_RECEIVER_SELF_PTR;
            }
            else if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_MUT)
            {
                parser_eat(parser, FRX_TOKEN_TYPE_KW_MUT);
                parser_eat(parser, FRX_TOKEN_TYPE_STAR);
                receiver = FRX_FUNC_RECEIVER_SELF_REF;
            }
            else
            {
                // TODO: Error, self must specify if pointer or reference!
            }

            if (parser_current_type(parser) == FRX_TOKEN_TYPE_COMMA)
            {
                parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
            }

            continue;
        }

        if (parser_match(parser, FRX_TOKEN_TYPE_ELLIPSIS))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_ELLIPSIS);
            is_variadic = FRX_TRUE;

            continue;
        }

        AST* param = func_param_parse(parser);
        list_add(&func_decl->params, param);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    parser_eat(parser, FRX_TOKEN_TYPE_ARROW);

    AST* return_type = type_specifier_parse(parser);

    AST* body = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LBRACE))
    {
        body = block_parse(parser);
        ast->span.hi = body->span.hi;
    }
    else
    {
        ast->span.hi = parser_current_span(parser).hi;
        parser_eat(parser, FRX_TOKEN_TYPE_SEMI);
    }

    parser_pop_scope(parser);

    func_decl_init(func_decl, name, external, receiver, is_variadic,
                   generic_params, return_type, body);

    parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_FUNC,
                         name, ast);

    return ast;
}

void func_decl_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    ctx->current_func_decl = ast;
    resolution_context_push_scope(ctx, attributes_table_lookup_scope(ast->id));

    switch (func_decl->receiver)
    {
        case FRX_FUNC_RECEIVER_NONE: break;
        case FRX_FUNC_RECEIVER_SELF_PTR: attributes_table_insert_func_receiver_type(ast->id, type_intern_ptr(symbol_infer_type(ctx->current_impl_block->impl_block.type_path->path.symbol), FRX_TRUE)); break;
        case FRX_FUNC_RECEIVER_SELF_REF: attributes_table_insert_func_receiver_type(ast->id, type_intern_ptr(symbol_infer_type(ctx->current_impl_block->impl_block.type_path->path.symbol), FRX_FALSE)); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }

    for (usize i = 0; i < list_size(&func_decl->params); ++i)
    {
        AST* param = list_get(&func_decl->params, i);
        func_param_resolve_early(param, ctx);
    }

    if (func_decl->return_type != NULL)
    {
        type_specifier_resolve(func_decl->return_type, ctx);
    }

    const Type* return_type = attributes_table_lookup_type(func_decl->return_type->id);
    const Type* type = type_intern_func(&func_decl->params, return_type, func_decl->is_variadic);
    attributes_table_insert_type(ast->id, type);

    resolution_context_pop_scope(ctx);
    ctx->current_func_decl = NULL;
}

void func_decl_resolve_late(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    ctx->current_func_decl = ast;
    resolution_context_push_scope(ctx, attributes_table_lookup_scope(ast->id));

    if (func_decl->body != NULL)
    {
        block_resolve(func_decl->body, ctx);
    }

    resolution_context_pop_scope(ctx);
    ctx->current_func_decl = NULL;
}

void func_decl_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    FRX_ASSERT(ctx != NULL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    if (func_decl->body != NULL)
    {
        block_sema(func_decl->body, ctx);
    }
}
