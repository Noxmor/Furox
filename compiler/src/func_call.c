#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "symbol_table.h"

static FuncCall* func_call_create(b8 error, const char* name)
{
    FRX_ASSERT(name != NULL);

    FuncCall* func_call = compiler_alloc(sizeof(FuncCall));

    func_call->error = error;
    func_call->id = symbol_intern(name);
    func_call->name = name;
    list_init(&func_call->args);

    return func_call;
}

FuncCall* func_call_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    const char* name = parser_current_token(parser)->identifier;
    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    FuncCall* func_call = func_call_create(error, name);

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
