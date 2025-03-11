#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "symbol_table.h"
#include "token.h"

#include <string.h>

static FuncCall* func_call_create(b8 error, const char* name, b8 external)
{
    FRX_ASSERT(name != NULL);

    FuncCall* func_call = compiler_alloc(sizeof(FuncCall));

    func_call->error = error;
    func_call->name = name;
    func_call->external = external;
    func_call->instantiation = NULL;
    list_init(&func_call->args);

    return func_call;
}

static GenericInstantiation* generic_params_instantiate(GenericParams* generic_params,
                                                      FuncParams* params, List* args)
{
    FRX_ASSERT(generic_params != NULL);

    FRX_ASSERT(params != NULL);

    FRX_ASSERT(args != NULL);

    GenericInstantiation* instantiation = compiler_alloc(sizeof(GenericInstantiation));
    list_init(&instantiation->concrete_types);

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        FuncParam* param = list_get(&params->params, i);
        if (param->type->kind != FRX_TYPE_KIND_UNRESOLVED)
        {
            continue;
        }

        b8 is_generic_param = FRX_FALSE;
        for (usize j = 0; j < list_size(&generic_params->params); ++j)
        {
            GenericParam* generic_param = list_get(&generic_params->params, j);
            if (generic_param->name == param->type->name)
            {
                is_generic_param = FRX_TRUE;
                break;
            }
        }

        if (!is_generic_param)
        {
            continue;
        }

        Expr* arg = list_get(args, i);
        TypeSpecifier* type = expr_infer_type(arg);
        list_add(&instantiation->concrete_types, type);
    }

    return instantiation;
}

FuncCall* func_call_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    const char* name = parser_current_token(parser)->identifier;
    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    FuncCall* func_call = func_call_create(error, name, parser->external);

    func_call->error |= parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&func_call->args))
        {
            func_call->error |= parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        Expr* arg = expr_parse(parser);
        list_add(&func_call->args, arg);
    }

    func_call->error |= parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return func_call;
}

void func_call_resolve(Parser* parser, FuncCall* func_call)
{
    FRX_ASSERT(func_call != NULL);

    if (func_call->error)
    {
        return;
    }

    if (func_call->external)
    {
        func_call->symbol = parser_lookup_symbol(parser, FRX_SYMBOL_TYPE_EXTERN_FUNC, func_call->name);
    }
    else
    {
        func_call->symbol = parser_lookup_symbol(parser, FRX_SYMBOL_TYPE_FUNC, func_call->name);
    }

    for (usize i = 0; i < list_size(&func_call->args); ++i)
    {
        Expr* arg = list_get(&func_call->args, i);
        expr_resolve(parser, arg);
    }

    if (func_call->symbol == NULL)
    {
        func_call->error = FRX_TRUE;
        SourceRange range; //TODO: Replace with correct range
        range.start.line = 0;
        range.start.column = 0;
        FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_UNRESOLVED_SYMBOL, FRX_DIAGNOSTIC_LVL_ERROR, range, func_call->name);
        parser_fail(parser);
    }
    else
    {
        FuncDef* func_def = func_call->symbol;
        if (func_def->generic_params != NULL)
        {
            GenericInstantiation* instantiation = generic_params_instantiate(func_def->generic_params, func_def->params, &func_call->args);
            list_add(&func_def->generic_instantiations, instantiation);
            func_call->instantiation = instantiation;
        }
    }
}

void func_call_sema(FuncCall* func_call)
{
    FRX_ASSERT(func_call != NULL);

    if (func_call->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&func_call->args); ++i)
    {
        Expr* arg = list_get(&func_call->args, i);
        expr_sema(arg);
    }
}

void func_call_codegen(FuncCall* func_call)
{
    FRX_ASSERT(func_call != NULL);
    FRX_ASSERT(!func_call->error);

    if (func_call->external || strcmp(func_call->name, "main") == 0)
    {
        codegen_write("%s(", func_call->name);
    }
    else
    {
        codegen_write("_FRX%s", func_call->name);
        if (func_call->instantiation != NULL)
        {
            GenericInstantiation* instantiation = func_call->instantiation;
            for (usize i = 0; i < list_size(&instantiation->concrete_types); ++i)
            {
                TypeSpecifier* type = list_get(&instantiation->concrete_types, i);
                if (type->kind == FRX_TYPE_KIND_PRIMITIVE)
                {
                    codegen_write("%s", token_type_to_str(type->primitive));
                }
                else if (type->kind == FRX_TYPE_KIND_STRUCT)
                {
                    codegen_write("%s", type->name);
                }
                else
                {
                    FRX_ASSERT(FRX_FALSE);
                }
            }
        }

        codegen_write("%p(", func_call->symbol);
    }

    for (usize i = 0; i < list_size(&func_call->args); ++i)
    {
        Expr* arg = list_get(&func_call->args, i);
        expr_codegen(arg);

        if (i + 1 < list_size(&func_call->args))
        {
            codegen_write(", ");
        }
    }

    codegen_write(")");
}
