#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static FuncParam* func_param_create(b8 error, const char* name, TypeSpecifier* type)
{
    FRX_ASSERT(name != NULL);
    FRX_ASSERT(type != NULL);

    FuncParam* param = compiler_alloc(sizeof(FuncParam));

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

static void func_param_codegen(FuncParam* param)
{
    FRX_ASSERT(param != NULL);
    FRX_ASSERT(!param->error);

    type_specifier_codegen(param->type);
    codegen_write(" %s", param->name);
}

static FuncParams* func_params_create(b8 error)
{
    FuncParams* params = compiler_alloc(sizeof(FuncParams));

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

void func_params_codegen(FuncParams* params)
{
    FRX_ASSERT(params != NULL);
    FRX_ASSERT(!params->error);

    codegen_write("(");

    if (!list_empty(&params->params))
    {
        FuncParam* param = list_get(&params->params, 0);
        func_param_codegen(param);
    }

    for (usize i = 1; i < list_size(&params->params); ++i)
    {
        codegen_write(", ");
        FuncParam* param = list_get(&params->params, i);
        func_param_codegen(param);
    }

    if (params->variadic)
    {
        codegen_write(", ...");
    }

    codegen_write(")");
}
