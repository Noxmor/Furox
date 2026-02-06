#include "assert.h"
#include "ast.h"
#include "parser.h"

AST* char_literal_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_CHAR_LIT);
    ASTCharLiteral* char_literal = &ast->char_literal;

    char_literal->value = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_CHAR_LIT);

    return ast;
}
