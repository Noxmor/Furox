#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void func_decl_init(ASTFuncDecl* func_decl, const char* name,
                           AST* generic_params, AST* params, AST* return_type)
{
    FRX_ASSERT(name != NULL);

    func_decl->name = name;
    func_decl->generic_params = generic_params;
    func_decl->params = params;
    func_decl->return_type = return_type;
}

AST* func_decl_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_DECL);
    ASTFuncDecl* func_decl = &ast->func_decl;

    ast->range.start = parser_current_location(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_FN))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    const char* name = parser_current_token(parser)->identifier;
    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    AST* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    AST* params = func_params_parse(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_ARROW))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    AST* return_type = type_specifier_parse(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_SEMI))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    func_decl_init(func_decl, name, generic_params, params, return_type);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void func_decl_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    if (func_decl->params != NULL)
    {
        func_params_resolve(func_decl->params, ctx);
    }

    if (func_decl->return_type != NULL)
    {
        type_specifier_resolve(func_decl->return_type, ctx);
    }
}

void func_decl_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_DECL);

    FRX_ASSERT(ctx != NULL);

    ASTFuncDecl* func_decl = &ast->func_decl;

    if (func_decl->params != NULL)
    {
        func_params_sema(func_decl->params, ctx);
    }

    if (func_decl->return_type != NULL)
    {
        type_specifier_sema(func_decl->return_type, ctx);
    }
}
