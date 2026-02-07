#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "type_system.h"

static AST* method_call_expr_create(const char* name, AST* callee)
{
    AST* ast = ast_create(FRX_AST_TYPE_METHOD_CALL_EXPR);

    ASTMethodCallExpr* method_call_expr = &ast->method_call_expr;

    method_call_expr->name = name;
    method_call_expr->callee = callee;
    list_init(&method_call_expr->args);
    method_call_expr->resolved_type = NULL;
    method_call_expr->symbol = NULL;

    return ast;
}

AST* method_call_expr_parse(Parser* parser, const char* name, AST* callee)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = method_call_expr_create(name, callee);
    ASTMethodCallExpr* method_call_expr = &ast->method_call_expr;

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        AST* arg = expr_parse(parser);
        list_add(&method_call_expr->args, arg);

        if (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return ast;
}

void method_call_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_METHOD_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTMethodCallExpr* method_call_expr = &ast->method_call_expr;

    if (method_call_expr->callee != NULL)
    {
        ast_resolve(method_call_expr->callee, ctx);
    }

    for (usize i = 0; i < list_size(&method_call_expr->args); ++i)
    {
        AST* arg = list_get(&method_call_expr->args, i);
        ast_resolve(arg, ctx);
    }
}

void method_call_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_METHOD_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTMethodCallExpr* method_call_expr = &ast->method_call_expr;

    for (usize i = 0; i < list_size(&method_call_expr->args); ++i)
    {
        AST* arg = list_get(&method_call_expr->args, i);
        ast_sema(arg, ctx);
    }

    if (method_call_expr->callee != NULL)
    {
        ast_sema(method_call_expr->callee, ctx);

        const Type* type = expr_infer_type(method_call_expr->callee);
        method_call_expr->symbol = type_lookup_method(type, method_call_expr->name);

        if (method_call_expr->symbol == NULL)
        {
            AST* base = method_call_expr->callee;
            const char* name = method_call_expr->name;
            List args = method_call_expr->args;

            ast->type = FRX_AST_TYPE_CALL_EXPR;
            ASTCallExpr* call_expr = &ast->call_expr;

            AST* field_expr = ast_create(FRX_AST_TYPE_FIELD_EXPR);
            field_expr->field_expr.field_name = name;
            field_expr->field_expr.base = base;
            field_expr->field_expr.resolved_type = type;

            call_expr->callee = field_expr;
            call_expr->args = args;
        }
        else
        {
            method_call_expr->resolved_type = ((ASTFuncDecl*)method_call_expr->symbol->data)->return_type->type_specifier.resolved_type;
        }

    }
}
