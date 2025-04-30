#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "codegen.h"

AST* break_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_BREAK_STMT);
    ast->range.start = parser_current_location(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_BREAK))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    ast->range.end = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);


    return ast;
}

void break_stmt_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BREAK_STMT);

    fprintf(ctx->source, "break;\n");
}
