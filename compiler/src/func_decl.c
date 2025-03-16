#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static FuncDecl* func_decl_create(const char* name, GenericParams* generic_params,
                                  FuncParams* params, TypeSpecifier* return_type)
{
    FRX_ASSERT(name != NULL);

    FuncDecl* func_decl = compiler_alloc_ast(sizeof(FuncDef));

    func_decl->name = name;
    func_decl->generic_params = generic_params;
    func_decl->params = params;
    func_decl->return_type = return_type;

    return func_decl;
}

FuncDecl* func_decl_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_FN);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    GenericParams* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    FuncParams* params = func_params_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_ARROW);

    TypeSpecifier* return_type = type_specifier_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    FuncDecl* func_decl = func_decl_create(name, generic_params, params, return_type);

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_EXTERN_FUNC, func_decl->name, func_decl);

    return func_decl;
}

void func_decl_resolve(Parser* parser, FuncDecl* func_decl)
{
    FRX_ASSERT(func_decl != NULL);

    if (func_decl->params != NULL)
    {
        func_params_resolve(parser, func_decl->params);
    }

    if (func_decl->return_type != NULL)
    {
        type_specifier_resolve(parser, func_decl->return_type);
    }
}

void func_decl_sema(FuncDecl* func_decl)
{
    FRX_ASSERT(func_decl != NULL);

    if (func_decl->params != NULL)
    {
        func_params_sema(func_decl->params);
    }

    if (func_decl->return_type != NULL)
    {
        type_specifier_sema(func_decl->return_type);
    }
}
