#include "assert.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"

AST* self_expr_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_SELF_EXPR);

    ast->span = parser_current_span(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_SELF_LOWER);

    return ast;
}

void self_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_SELF_EXPR);

    FRX_ASSERT(ctx != NULL);

    const Type* type = attributes_table_lookup_func_receiver_type(ctx->current_func_decl->id);
    attributes_table_insert_type(ast->id, type);
}
