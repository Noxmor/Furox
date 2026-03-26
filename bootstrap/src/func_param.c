#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "early_resolution.h"

AST* func_param_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_FUNC_PARAM);
    ASTFuncParam* param = &ast->func_param;

    ast->span.lo = parser_current_span(parser).lo;

    param->name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    param->type = type_specifier_parse(parser);

    ast->span.hi = param->type->span.hi;

    parser_insert_symbol(parser, FRX_SYMBOL_VISIBILITY_PRIVATE,
                         FRX_SYMBOL_TYPE_PARAM, param->name, &ast->func_param);

    return ast;
}

void func_param_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FUNC_PARAM);

    ASTFuncParam* param = &ast->func_param;

    if (param->type != NULL)
    {
        type_specifier_resolve(param->type, ctx);
    }
}
