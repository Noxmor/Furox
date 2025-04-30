#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "parser.h"

AST* continue_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_CONTINUE_STMT);
    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_CONTINUE);
    ast->range.end = parser_current_location(parser);
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return ast;
}

void continue_stmt_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CONTINUE_STMT);

    FRX_ASSERT(ctx != NULL);

    fprintf(ctx->source, "continue;\n");
}
