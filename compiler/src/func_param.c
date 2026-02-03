#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static AST* ast_func_param_create(const char* name, AST* type)
{
    FRX_ASSERT(name != NULL);
    FRX_ASSERT(type != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_FUNC_PARAM);
    ASTFuncParam* param = &ast->func_param;

    param->name = name;
    param->type = type;

    return ast;
}

AST* func_param_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* type = type_specifier_parse(parser);
    AST* ast = ast_func_param_create(name, type);

    parser_insert_symbol(parser, FRX_SYMBOL_VISIBILITY_PRIVATE,
                         FRX_SYMBOL_TYPE_PARAM, name, &ast->func_param);

    return ast;
}

void func_param_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    ASTFuncParam* param = &ast->func_param;

    if (param->type != NULL)
    {
        type_specifier_resolve(param->type, ctx);
    }
}
