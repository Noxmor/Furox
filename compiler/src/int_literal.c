#include "ast.h"
#include "parser.h"

static void int_literal_init(ASTIntLiteral* literal, u64 value)
{
    literal->value = value;
}

AST* int_literal_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_INT_LIT);
    ASTIntLiteral* literal = &ast->int_literal;

    ast->range.start = parser_current_location(parser);

    u64 value = parser_current_token(parser)->int_literal;
    int_literal_init(literal, value);

    parser_eat(parser, FRX_TOKEN_TYPE_INT_LIT);

    ast->range.end = parser_current_location(parser);

    return ast;
}
