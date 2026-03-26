#include "assert.h"
#include "ast.h"
#include "module.h"
#include "parser.h"
#include "early_resolution.h"

static void use_stmt_init(ASTUseStmt* use_stmt, AST* use_tree)
{
    FRX_ASSERT(use_stmt != NULL);

    FRX_ASSERT(use_tree != NULL);

    use_stmt->use_tree = use_tree;
}

AST* use_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_USE_STMT);
    ASTUseStmt* use_stmt = &ast->use_stmt;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_USE);

    AST* use_tree = use_tree_parse(parser);

    use_stmt_init(use_stmt, use_tree);

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return ast;
}

void use_stmt_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_USE_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTUseStmt* use_stmt = &ast->use_stmt;

    use_tree_resolve_early(use_stmt->use_tree, ctx);
}
