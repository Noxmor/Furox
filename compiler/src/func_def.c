#include "assert.h"
#include "ast.h"
#include "hir.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "token.h"

#include <string.h>

static void func_def_init(ASTFuncDef* func_def, const char* name,
                          AST* generic_params, AST* params, AST* return_type,
                          AST* body)
{
    FRX_ASSERT(name != NULL);

    func_def->name = name;
    func_def->generic_params = generic_params;
    list_init(&func_def->generic_instantiations);
    func_def->params = params;
    func_def->return_type = return_type;
    func_def->body = body;
    func_def->symbol = NULL;
}

AST* func_def_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_DEF);
    ASTFuncDef* func_def = &ast->func_def;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_FN);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    AST* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    AST* params = func_params_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_ARROW);

    AST* return_type = type_specifier_parse(parser);

    func_def_init(func_def, name, generic_params, params, return_type,
                  scope_parse(parser));

    ast->range.end = parser_current_location(parser);

    func_def->symbol = parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_FUNC, func_def->name, NULL);

    return ast;
}

void func_def_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DEF);

    ASTFuncDef* func_def = &ast->func_def;

    Symbol* symbol = func_def->symbol;
    FuncDef* data = func_def_create(func_def->name);
    symbol->data = data;

    if (func_def->params != NULL)
    {
        data->params = func_params_resolve(func_def->params, ctx);
    }

    if (func_def->return_type != NULL)
    {
        data->return_type = type_specifier_resolve(func_def->return_type, ctx);
    }

    if (func_def->body != NULL)
    {
        scope_resolve(func_def->body, ctx);
    }

    data->body = func_def->body;
}

void func_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DEF);

    FRX_ASSERT(ctx != NULL);

    ASTFuncDef* func_def = &ast->func_def;

    if (func_def->params != NULL)
    {
        func_params_sema(func_def->params, ctx);
    }

    if (func_def->return_type != NULL)
    {
        type_specifier_sema(func_def->return_type, ctx);
    }

    if (func_def->body != NULL)
    {
        scope_sema(func_def->body, ctx);
    }
}
