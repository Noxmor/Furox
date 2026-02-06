#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "symbol.h"
#include "type_system.h"

static void func_decl_init(ASTFuncDecl* func_decl, const char* name, b8 external,
                           b8 is_variadic, AST* generic_params, AST* return_type,
                           AST* body)
{
    FRX_ASSERT(name != NULL);

    func_decl->name = name;
    func_decl->external = external;
    func_decl->generic_params = generic_params;
    func_decl->is_variadic = is_variadic;
    func_decl->return_type = return_type;
    func_decl->body = body;
    func_decl->resolved_type = NULL;
}

AST* func_decl_parse(Parser* parser, SymbolVisibility visibility)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_DECL);
    ASTFuncDecl* func_decl = &ast->func_decl;
    list_init(&func_decl->params);
    func_decl->scope = parser_push_scope(parser);

    ast->range.start = parser_current_location(parser);

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

    const char* name = parser_current_token(parser)->identifier;
    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    AST* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    b8 is_variadic = FRX_FALSE;

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&func_decl->params))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
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
        body = scope_parse(parser);
    }
    else
    {
        parser_eat(parser, FRX_TOKEN_TYPE_SEMI);
    }

    parser_pop_scope(parser);

    func_decl_init(func_decl, name, external, is_variadic, generic_params, return_type, body);

    ast->range.end = parser_current_location(parser);

    parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_FUNC,
                         name, func_decl);

    return ast;
}

void func_decl_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    resolution_context_push_scope(ctx, func_decl->scope);

    for (usize i = 0; i < list_size(&func_decl->params); ++i)
    {
        AST* param = list_get(&func_decl->params, i);
        func_param_resolve(param, ctx);
    }

    if (func_decl->return_type != NULL)
    {
        type_specifier_resolve(func_decl->return_type, ctx);
    }

    if (func_decl->body != NULL)
    {
        scope_resolve(func_decl->body, ctx);
    }

    func_decl->resolved_type = type_create_func(&func_decl->params, func_decl->return_type, func_decl->is_variadic);

    resolution_context_pop_scope(ctx);
}

void func_decl_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    FRX_ASSERT(ctx != NULL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    if (func_decl->body != NULL)
    {
        scope_sema(func_decl->body, ctx);
    }
}
