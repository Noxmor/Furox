#include "assert.h"
#include "ast.h"
#include "hir.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "symbol_table.h"

static AST* ast_func_param_create(const char* name, AST* type)
{
    FRX_ASSERT(name != NULL);
    FRX_ASSERT(type != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_FUNC_PARAM);
    ASTFuncParam* param = &ast->func_param;

    param->name = name;
    param->type = type;

    return ast;
}

static AST* func_param_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* type = type_specifier_parse(parser);
    AST* ast = ast_func_param_create(name, type);

    return ast;
}

static FuncParam* func_param_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    ASTFuncParam* param = &ast->func_param;

    Type* type = NULL;

    if (param->type != NULL)
    {
        type = type_specifier_resolve(param->type, ctx);
    }

    FuncParam* func_param = func_param_create(param->name, type);

    symbol_table_insert(&ctx->src_file->symbol_table, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_PARAM, func_param->name, func_param);

    return func_param;
}

static void func_param_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    FRX_ASSERT(ctx != NULL);

    ASTFuncParam* param = &ast->func_param;

    if (param->type != NULL)
    {
        type_specifier_sema(param->type, ctx);
    }
}

static void func_params_init(ASTFuncParams* params)
{
    list_init(&params->params);
    params->variadic = FRX_FALSE;
}

AST* func_params_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_PARAMS);
    ASTFuncParams* params = &ast->func_params;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    func_params_init(params);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&params->params))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        if (parser_match(parser, FRX_TOKEN_TYPE_ELLIPSIS))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_ELLIPSIS);
            params->variadic = FRX_TRUE;

            break;
        }

        AST* param = func_param_parse(parser);
        list_add(&params->params, param);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return ast;
}

FuncParams* func_params_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAMS);

    ASTFuncParams* params = &ast->func_params;

    FuncParams* func_params = func_params_create(params->variadic);

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        AST* param = list_get(&params->params, i);
        FuncParam* func_param = func_param_resolve(param, ctx);
        func_params_add_param(func_params, func_param);
    }

    return func_params;
}

void func_params_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAMS);

    FRX_ASSERT(ctx != NULL);

    ASTFuncParams* params = &ast->func_params;

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        AST* param = list_get(&params->params, i);
        func_param_sema(param, ctx);
    }
}
