#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static FuncParam* func_param_create(b8 error, const char* name, TypeSpecifier* type)
{
    FRX_ASSERT(name != NULL);
    FRX_ASSERT(type != NULL);

    FuncParam* param = compiler_alloc_ast(sizeof(FuncParam));

    param->error = error;
    param->name = name;
    param->type = type;

    return param;
}

static FuncParam* func_param_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    const char* name = parser_current_token(parser)->identifier;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    error |= parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    TypeSpecifier* type = type_specifier_parse(parser);

    return func_param_create(error, name, type);
}

static void func_param_resolve(Parser* parser, FuncParam* param)
{
    FRX_ASSERT(param != NULL);

    if (param->error)
    {
        return;
    }

    if (param->type != NULL)
    {
        type_specifier_resolve(parser, param->type);
    }
}

static void func_param_sema(FuncParam* param)
{
    FRX_ASSERT(param != NULL);

    if (param->error)
    {
        return;
    }

    if (param->type != NULL)
    {
        type_specifier_sema(param->type);
    }
}

static FuncParams* func_params_create(b8 error)
{
    FuncParams* params = compiler_alloc_ast(sizeof(FuncParams));

    params->error = error;
    list_init(&params->params);
    params->variadic = FRX_FALSE;

    return params;
}

FuncParams* func_params_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    FuncParams* params = func_params_create(error);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&params->params))
        {
            error |= parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        if (parser_match(parser, FRX_TOKEN_TYPE_ELLIPSIS))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_ELLIPSIS);
            params->variadic = FRX_TRUE;

            break;
        }

        FuncParam* param = func_param_parse(parser);
        list_add(&params->params, param);
    }

    params->error |= parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return params;
}

void func_params_resolve(Parser* parser, FuncParams* params)
{
    FRX_ASSERT(params != NULL);

    if (params->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        FuncParam* param = list_get(&params->params, i);
        func_param_resolve(parser, param);
    }
}
void func_params_sema(FuncParams* params)
{
    FRX_ASSERT(params != NULL);

    if (params->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        FuncParam* param = list_get(&params->params, i);
        func_param_sema(param);
    }
}
