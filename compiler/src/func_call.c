#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "symbol_table.h"

static FuncCall* func_call_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    FuncCall* func_call = compiler_alloc(sizeof(FuncCall));

    func_call->id = symbol_intern(name);
    func_call->name = name;
    list_init(&func_call->args);

    return func_call;
}

FuncCall* func_call_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;
    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        return NULL;
    }

    FuncCall* func_call = func_call_create(name);

    if (parser_eat(parser, FRX_TOKEN_TYPE_LPAREN))
    {
        return NULL;
    }

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&func_call->args))
        {
            if (parser_eat(parser, FRX_TOKEN_TYPE_COMMA))
            {
                return NULL;
            }
        }

        Expr* arg = expr_parse(parser);
        if (arg != NULL)
        {
            list_add(&func_call->args, arg);
        }
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        return NULL;
    }

    return func_call;
}

void func_call_resolve(Parser* parser, FuncCall* func_call)
{
    FRX_ASSERT(func_call != NULL);

    func_call->symbol = parser_lookup_symbol(parser, func_call->id);

    for (usize i = 0; i < list_size(&func_call->args); ++i)
    {
        Expr* arg = list_get(&func_call->args, i);
        expr_resolve(parser, arg);
    }
}

void func_call_sema(FuncCall* func_call)
{
    FRX_ASSERT(func_call != NULL);

    for (usize i = 0; i < list_size(&func_call->args); ++i)
    {
        Expr* arg = list_get(&func_call->args, i);
        expr_sema(arg);
    }
}

void func_call_codegen(FuncCall* func_call)
{
    FRX_ASSERT(func_call != NULL);

    codegen_write("%s(", func_call->name);

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
