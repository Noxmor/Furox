#include "assert.h"
#include "ast.h"
#include "module.h"
#include "parser.h"
#include "resolution.h"

static void use_stmt_init(ASTUseStmt* use_stmt, const char* name)
{
    FRX_ASSERT(use_stmt != NULL);

    list_init(&use_stmt->path_segments);
    use_stmt->symbol_name = name;
}

AST* use_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_USE_STMT);
    ASTUseStmt* use_stmt = &ast->use_stmt;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_USE);

    const char* symbol_name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    use_stmt_init(use_stmt, symbol_name);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);

        list_add(&use_stmt->path_segments, (void*)use_stmt->symbol_name);
        use_stmt->symbol_name = parser_current_token(parser)->identifier;

        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    //list_add(&parser->use_stmts, use_stmt);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void use_stmt_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_USE_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTUseStmt* use_stmt = &ast->use_stmt;

    // TODO: Implement
    (void)use_stmt;
    (void)ctx;
}
