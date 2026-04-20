#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void if_stmt_init(ASTIfStmt* if_stmt, AST* condition, AST* then_block,
                         AST* else_stmt)
{
    if_stmt->condition = condition;
    if_stmt->then_block = then_block;
    if_stmt->else_stmt = else_stmt;
}

AST* if_stmt_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_IF_STMT);
    ASTIfStmt* if_stmt = &ast->if_stmt;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IF);

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    AST* condition = expr_parse(parser);

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    AST* if_block = block_parse(parser);
    AST* else_stmt = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_ELSE))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_ELSE);

        if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_IF
            || parser_current_type(parser) == FRX_TOKEN_TYPE_LBRACE)
        {
            else_stmt = stmt_parse(parser);
        }

    }

    if_stmt_init(if_stmt, condition, if_block, else_stmt);

    return ast;
}

void if_stmt_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    ASTIfStmt* if_stmt = &ast->if_stmt;

    if (if_stmt->condition != NULL)
    {
        ast_resolve(if_stmt->condition, ctx);
    }

    if (if_stmt->then_block != NULL)
    {
        block_resolve(if_stmt->then_block, ctx);
    }

    if (if_stmt->else_stmt != NULL)
    {
        ast_resolve(if_stmt->else_stmt, ctx);
    }
}

void if_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTIfStmt* if_stmt = &ast->if_stmt;

    if (if_stmt->condition != NULL)
    {
        ast_sema(if_stmt->condition, ctx);
    }

    if (if_stmt->then_block != NULL)
    {
        block_sema(if_stmt->then_block, ctx);
    }

    if (if_stmt->else_stmt != NULL)
    {
        ast_sema(if_stmt->else_stmt, ctx);
    }
}
