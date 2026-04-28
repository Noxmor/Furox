#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "type_system.h"

static AST* member_call_expr_create(Parser* parser, const char* name, AST* callee)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_MEMBER_CALL_EXPR);

    ASTMemberCallExpr* member_call_expr = &ast->member_call_expr;

    member_call_expr->name = name;
    member_call_expr->callee = callee;
    list_init(&member_call_expr->args);
    member_call_expr->symbol = NULL;

    return ast;
}

AST* member_call_expr_parse(Parser* parser, const char* name, AST* callee)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = member_call_expr_create(parser, name, callee);
    ASTMemberCallExpr* member_call_expr = &ast->member_call_expr;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        AST* arg = expr_parse(parser);
        list_add(&member_call_expr->args, arg);

        if (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return ast;
}

void member_call_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_MEMBER_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTMemberCallExpr* member_call_expr = &ast->member_call_expr;

    if (member_call_expr->callee != NULL)
    {
        ast_resolve(member_call_expr->callee, ctx);
    }

    for (usize i = 0; i < list_size(&member_call_expr->args); ++i)
    {
        AST* arg = list_get(&member_call_expr->args, i);
        ast_resolve(arg, ctx);
    }

    const Type* type = expr_infer_type(member_call_expr->callee);
    while (type->kind == FRX_TYPE_KIND_PTR)
    {
        type = type->ptr.base;
    }

    member_call_expr->symbol = type_lookup_method(type, member_call_expr->name);

    if (member_call_expr->symbol == NULL)
    {
        List* fields = &type->strct.symbol->data->struct_def.fields;
        type = NULL;

        for (usize i = 0; i < list_size(fields); ++i)
        {
            AST* field = list_get(fields, i);
            if (field->struct_field.name == member_call_expr->name)
            {
                type = attributes_table_lookup_type(field->struct_field.type->id);
                type = type->func.return_type;
                attributes_table_insert_type(ast->id, type);

                break;
            }
        }

        if (type == NULL)
        {
            resolution_context_fail(ctx);
        }
    }
    else
    {
        type = attributes_table_lookup_type(member_call_expr->symbol->data->func_decl.return_type->id);
        attributes_table_insert_type(ast->id, type);
    }
}

void member_call_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_MEMBER_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTMemberCallExpr* member_call_expr = &ast->member_call_expr;

    for (usize i = 0; i < list_size(&member_call_expr->args); ++i)
    {
        AST* arg = list_get(&member_call_expr->args, i);
        ast_sema(arg, ctx);
    }

    if (member_call_expr->callee != NULL)
    {
        ast_sema(member_call_expr->callee, ctx);
    }
}
