#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

AST* defer_stmt_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_DEFER_STMT);
    ASTDeferStmt* defer_stmt = &ast->defer_stmt;

    ast->span.lo = parser_current_span(parser).lo;
    parser_eat(parser, FRX_TOKEN_TYPE_KW_DEFER);

    defer_stmt->stmt = stmt_parse(parser);

    ast->span.hi = defer_stmt->stmt->span.hi;

    attributes_table_insert_scope(ast->id, parser->current_scope);

    return ast;
}

void defer_stmt_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_DEFER_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTDeferStmt* defer_stmt = &ast->defer_stmt;

    if (defer_stmt->stmt != NULL)
    {
        ast_resolve(defer_stmt->stmt, ctx);
    }
}

void defer_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_DEFER_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTDeferStmt* defer_stmt = &ast->defer_stmt;

    if (defer_stmt->stmt != NULL)
    {
        ast_sema(defer_stmt->stmt, ctx);
    }
}
