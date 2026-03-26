#include "assert.h"
#include "ast.h"
#include "parser.h"

AST* string_literal_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_STRING_LIT);
    ASTStringLiteral* string_literal = &ast->string_literal;

    ast->span = parser_current_span(parser);

    string_literal->value = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_STR_LIT);

    return ast;
}
