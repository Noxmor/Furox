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

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_STRUCT_LIT_FIELD);
    ASTStructLiteralField* field = &ast->struct_literal_field;

    ast->span.lo = parser_current_span(parser).lo;

    const char* name = parse_ident(parser);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* value = expr_parse(parser);

    ast->span.hi = parser_current_span(parser).hi;

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

static void struct_literal_init(ASTStructLiteral* literal, AST* path)
{
    FRX_ASSERT(literal != NULL);

    FRX_ASSERT(path != NULL);

    literal->path = path;
    list_init(&literal->fields);
}

AST* struct_literal_parse(Parser* parser, AST* path)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(path != NULL);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_STRUCT_LIT);
    ASTStructLiteral* literal = &ast->struct_literal;

    ast->span.lo = parser_current_span(parser).lo;

    struct_literal_init(literal, path);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        list_add(&literal->fields, struct_literal_field_parse(parser));
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    return ast;
}

void struct_literal_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_LIT);

    FRX_ASSERT(ctx != NULL);

    ASTStructLiteral* literal = &ast->struct_literal;

    path_resolve(literal->path, ctx);

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
