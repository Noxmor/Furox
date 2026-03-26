#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "type_system.h"

static void int_literal_init(ASTIntLiteral* literal, u64 value)
{
    literal->value = value;
    literal->resolved_type = NULL;
}

AST* int_literal_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_INT_LIT);
    ASTIntLiteral* literal = &ast->int_literal;

    ast->span = parser_current_span(parser);

    u64 value = parser_current_token(parser)->int_literal;
    int_literal_init(literal, value);

    parser_eat(parser, FRX_TOKEN_TYPE_INT_LIT);

    return ast;
}

void int_literal_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_INT_LIT);

    FRX_ASSERT(ctx != NULL);

    // TODO: Resolve type to i32 by default or smallest type to fit the value
    ASTIntLiteral* literal = &ast->int_literal;

    literal->resolved_type = type_intern_primitive(FRX_TOKEN_TYPE_KW_I32);
}
