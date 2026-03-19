#include "assert.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void struct_literal_field_init(ASTStructLiteralField* field,
                                      const char* name, AST* value)
{
    FRX_ASSERT(field != NULL);

    FRX_ASSERT(name != NULL);

    field->name = name;
    field->value = value;
}

AST* struct_literal_field_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_LIT_FIELD);
    ASTStructLiteralField* field = &ast->struct_literal_field;


    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* value = expr_parse(parser);
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    struct_literal_field_init(field, name, value);

    return ast;
}

void struct_literal_field_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_LIT_FIELD);

    FRX_ASSERT(ctx != NULL);

    ASTStructLiteralField* field = &ast->struct_literal_field;

    ast_resolve(field->value, ctx);
}

void struct_literal_field_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_LIT_FIELD);

    FRX_ASSERT(ctx != NULL);

    ASTStructLiteralField* field = &ast->struct_literal_field;

    ast_sema(field->value, ctx);

}

static void struct_literal_init(ASTStructLiteral* literal, AST* path_expr)
{
    FRX_ASSERT(literal != NULL);

    FRX_ASSERT(path_expr != NULL);

    literal->path_expr = path_expr;
    list_init(&literal->fields);
}

AST* struct_literal_parse(Parser* parser, AST* path_expr)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(path_expr != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_LIT);
    ASTStructLiteral* literal = &ast->struct_literal;

    struct_literal_init(literal, path_expr);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        list_add(&literal->fields, struct_literal_field_parse(parser));
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    return ast;
}

void struct_literal_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_LIT);

    FRX_ASSERT(ctx != NULL);

    ASTStructLiteral* literal = &ast->struct_literal;

    path_expr_resolve(literal->path_expr, ctx);

    for (usize i = 0; i < list_size(&literal->fields); ++i)
    {
        AST* field = list_get(&literal->fields, i);
        struct_literal_field_resolve(field, ctx);
    }
}

void struct_literal_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_LIT);

    FRX_ASSERT(ctx != NULL);

    ASTStructLiteral* literal = &ast->struct_literal;

    for (usize i = 0; i < list_size(&literal->fields); ++i)
    {
        AST* field = list_get(&literal->fields, i);
        struct_literal_field_sema(field, ctx);
    }
}
