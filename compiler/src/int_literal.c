#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "parser.h"

static void int_literal_init(IntLiteral* literal, u64 value)
{
    literal->value = value;
}

AST* int_literal_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_INT_LIT);
    IntLiteral* literal = &ast->int_literal;

    ast->range.start = parser_current_location(parser);

    u64 value = parser_current_token(parser)->int_literal;
    int_literal_init(literal, value);

    parser_eat(parser, FRX_TOKEN_TYPE_INT_LIT);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void int_literal_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_INT_LIT);

    FRX_ASSERT(ctx != NULL);

    IntLiteral* literal = &ast->int_literal;

    fprintf(ctx->source, "%zu", literal->value);
}
