#include "assert.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

AST* static_parse(Parser* parser, SymbolVisibility visibility)
{
    FRX_ASSERT(visibility < FRX_SYMBOL_VISIBILITY_COUNT);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_STATIC);
    ASTStatic* static_node = &ast->static_node;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_STATIC);

    static_node->name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    AST* type = NULL;

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_COLON)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_COLON);

        type = type_specifier_parse(parser);
    }

    static_node->value = NULL;

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_EQ)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_EQ);

        static_node->value = expr_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_STATIC,
                         static_node->name, ast);

    return ast;
}

void static_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STATIC);

    FRX_ASSERT(ctx != NULL);

    ASTStatic* static_node = &ast->static_node;

    if (static_node->type != NULL)
    {
        type_specifier_resolve(static_node->type, ctx);

        const Type* type = attributes_table_lookup_type(static_node->type->id);
        attributes_table_insert_type(ast->id, type);
    }

    if (static_node->value != NULL)
    {
        ast_resolve(static_node->value, ctx);

        if (static_node->type == NULL)
        {
            const Type* type = expr_infer_type(static_node->value);
            attributes_table_insert_type(ast->id, type);
        }
    }
}

void static_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STATIC);

    FRX_ASSERT(ctx != NULL);

    ASTStatic* static_node = &ast->static_node;

    if (static_node->value != NULL)
    {
        ast_sema(static_node->value, ctx);
    }
}
