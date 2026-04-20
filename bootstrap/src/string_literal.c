#include "assert.h"
#include "ast.h"
#include "parser.h"

AST* string_literal_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_STRING_LIT);
    ASTStringLiteral* string_literal = &ast->string_literal;

    ast->span = parser_current_span(parser);

    string_literal->value = parse_string_literal(parser);

    return ast;
}
