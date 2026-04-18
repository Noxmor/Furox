#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"
#include "type_system.h"

static void int_literal_init(ASTIntLiteral* literal, u64 value)
{
    literal->value = value;
}

AST* int_literal_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_INT_LIT);
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

    const Type* type = type_intern_primitive(FRX_TOKEN_TYPE_KW_I32);
    attributes_table_insert_type(ast->id, type);
}
