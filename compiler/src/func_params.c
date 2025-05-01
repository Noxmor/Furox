#include "assert.h"
#include "ast.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static AST* func_param_create(const char* name, AST* type)
{
    FRX_ASSERT(name != NULL);
    FRX_ASSERT(type != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_FUNC_PARAM);
    FuncParam* param = &ast->func_param;

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
    AST* ast = func_param_create(name, type);

    // FIXME: Add to correct symbol table
    parser_insert_symbol(parser, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_PARAM, name, ast);

    return ast;
}

static void func_param_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    FuncParam* param = &ast->func_param;

    if (param->type != NULL)
    {
        type_specifier_resolve(param->type, parser);
    }
}

static void func_param_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    FRX_ASSERT(ctx != NULL);

    FuncParam* param = &ast->func_param;

    if (param->type != NULL)
    {
        type_specifier_sema(param->type, ctx);
    }
}

static void func_params_init(FuncParams* params)
{
    list_init(&params->params);
    params->variadic = FRX_FALSE;
}

AST* func_params_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_PARAMS);
    FuncParams* params = &ast->func_params;

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

void func_params_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAMS);

    FuncParams* params = &ast->func_params;

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        AST* param = list_get(&params->params, i);
        func_param_resolve(param, parser);
    }
}

void func_params_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAMS);

    FRX_ASSERT(ctx != NULL);

    FuncParams* params = &ast->func_params;

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        AST* param = list_get(&params->params, i);
        func_param_sema(param, ctx);
    }
}

static void func_param_codegen(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    FRX_ASSERT(f != NULL);

    FuncParam* param  = &ast->func_param;

    type_specifier_codegen(param->type, f);
    fprintf(f, " %s", param->name);
}

void func_params_codegen(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAMS);

    FRX_ASSERT(f != NULL);

    FuncParams* params = &ast->func_params;

    fprintf(f, "(");

    if (list_empty(&params->params))
    {
        fprintf(f, "void");
    }

    for (usize i = 0; i < list_size(&params->params); ++i)
    {
        if (i > 0)
        {
            fprintf(f, ", ");
        }

        AST* param = list_get(&params->params, i);
        func_param_codegen(param, f);
    }

    if (params->variadic)
    {
        fprintf(f, ", ...");
    }

    fprintf(f, ")");
}
