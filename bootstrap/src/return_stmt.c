#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void return_stmt_init(ASTReturnStmt* return_stmt, AST* value)
{
    return_stmt->value = value;
}

AST* return_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_RETURN_STMT);
    ASTReturnStmt* return_stmt = &ast->return_stmt;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_RETURN);

    AST* value = NULL;

    if (!parser_match(parser, FRX_TOKEN_TYPE_SEMI))
    {
        value = expr_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return_stmt_init(return_stmt, value);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void return_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_RETURN_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTReturnStmt* return_stmt = &ast->return_stmt;

    if (return_stmt->value != NULL)
    {
        ast_sema(return_stmt->value, ctx);
    }
}
