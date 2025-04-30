#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "token.h"

#include <string.h>

static void func_def_init(FuncDef* func_def, const char* name,
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
}

AST* func_def_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_DEF);
    FuncDef* func_def = &ast->func_def;

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

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_FUNC,
                         func_def->name, func_def);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void func_def_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DEF);

    FuncDef* func_def = &ast->func_def;

    if (func_def->params != NULL)
    {
        func_params_resolve(func_def->params, parser);
    }

    if (func_def->return_type != NULL)
    {
        type_specifier_resolve(func_def->return_type, parser);
    }

    if (func_def->body != NULL)
    {
        scope_resolve(func_def->body, parser);
    }
}

void func_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DEF);

    FRX_ASSERT(ctx != NULL);

    FuncDef* func_def = &ast->func_def;

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

void func_def_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DEF);

    FRX_ASSERT(ctx != NULL);

    FuncDef* func_def = &ast->func_def;

    type_specifier_codegen(func_def->return_type, ctx->header);
    fprintf(ctx->header, " %s", func_def->name);

    if (strcmp(func_def->name, "main") != 0)
    {
        fprintf(ctx->header, "%p", func_def);
    }

    func_params_codegen(func_def->params, ctx->header);
    fprintf(ctx->header, ";\n");

    type_specifier_codegen(func_def->return_type, ctx->source);
    fprintf(ctx->source, " %s", func_def->name);

    if (strcmp(func_def->name, "main") != 0)
    {
        fprintf(ctx->source, "%p", func_def);
    }

    func_params_codegen(func_def->params, ctx->source);
    fprintf(ctx->source, "\n");

    scope_codegen(func_def->body, ctx);
}
