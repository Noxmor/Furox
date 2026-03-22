#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"

AST* type_alias_parse(Parser* parser, SymbolVisibility visibility)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_TYPE_ALIAS);
    ASTTypeAlias* type_alias = &ast->type_alias;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_TYPE);

    type_alias->name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    parser_eat(parser, FRX_TOKEN_TYPE_EQ);

    type_alias->type = type_specifier_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_TYPE_ALIAS, type_alias->name, type_alias);

    return ast;
}

void type_alias_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TYPE_ALIAS);

    FRX_ASSERT(ctx != NULL);

    ASTTypeAlias* type_alias = &ast->type_alias;

    type_specifier_resolve(type_alias->type, ctx);
}
