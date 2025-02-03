#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static FuncDef* func_def_create(const char* name, TypeSpecifier* return_type,
                                Scope* body)
{
    FRX_ASSERT(name != NULL);

    FuncDef* func_def = compiler_alloc(sizeof(FuncDef));

    func_def->name = name;
    func_def->return_type = return_type;
    func_def->body = body;

    return func_def;
}

FuncDef* func_def_parse(Parser* parser)
{
    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_FN))
    {
        return NULL;
    }

    const char* name = parser_current_token(parser)->identifier;
    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        return NULL;
    }

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);
    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    if (parser_eat(parser, FRX_TOKEN_TYPE_ARROW))
    {
        return NULL;
    }

    TypeSpecifier* return_type = type_specifier_parse(parser);

    FuncDef* func_def = func_def_create(name, return_type, scope_parse(parser));

    return func_def;
}

void func_def_sema(FuncDef* func_def)
{
    FRX_ASSERT(func_def != NULL);

    if (func_def->return_type != NULL)
    {
        type_specifier_sema(func_def->return_type);
    }

    if (func_def->body != NULL)
    {
        scope_sema(func_def->body);
    }
}

void func_def_codegen(FuncDef* func_def)
{
    FRX_ASSERT(func_def != NULL);

    type_specifier_codegen(func_def->return_type);
    codegen_write(" %s()\n", func_def->name);

    scope_codegen(func_def->body);
}
