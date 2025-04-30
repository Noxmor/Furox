#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

AST* expr_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_EXPR_STMT);
    ast->range.start = parser_current_location(parser);
    ExprStmt* expr_stmt = &ast->expr_stmt;

    expr_stmt->expr = expr_parse(parser);

    ast->range.end = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return ast;
}

void expr_stmt_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    ExprStmt* expr_stmt = &ast->expr_stmt;

    ast_resolve(expr_stmt->expr, parser);
}

void expr_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    FRX_ASSERT(ctx != NULL);

    ExprStmt* expr_stmt = &ast->expr_stmt;

    ast_sema(expr_stmt->expr, ctx);
}

void expr_stmt_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_EXPR_STMT);

    FRX_ASSERT(ctx != NULL);

    ExprStmt* expr_stmt = &ast->expr_stmt;

    ast_codegen(expr_stmt->expr, ctx);

    fprintf(ctx->source, ";\n");
}
