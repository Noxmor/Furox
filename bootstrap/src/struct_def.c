#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "symbol.h"

static void struct_field_init(ASTStructField* field, const char* name,
                              SymbolVisibility visibility, AST* type)
{
    field->name = name;
    field->visibility = visibility;
    field->type = type;
}

static void struct_def_init(ASTStructDef* struct_def, const char* name,
                            StructKind kind, AST* generic_params)
{
    struct_def->name = name;
    struct_def->kind = kind;
    struct_def->generic_params = generic_params;
    list_init(&struct_def->fields);
    list_init(&struct_def->instantiated_types);
}

AST* struct_field_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_FIELD);
    ASTStructField* field = &ast->struct_field;

    ast->range.start = parser_current_location(parser);

    SymbolVisibility visibility = FRX_SYMBOL_VISIBILITY_PRIVATE;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_PUB)
    {
        visibility = FRX_SYMBOL_VISIBILITY_PUBLIC;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_PUB);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_MOD)
    {
        visibility = FRX_SYMBOL_VISIBILITY_MODULE;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);
    }

    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* type = type_specifier_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    struct_field_init(field, name, visibility, type);

    ast->range.end = parser_current_location(parser);

    return ast;
}

static void struct_field_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_FIELD);

    ASTStructField* field = &ast->struct_field;

    type_specifier_resolve(field->type, ctx);
}

AST* struct_def_parse(Parser* parser, SymbolVisibility visibility)
{
    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_DEF);
    ASTStructDef* struct_def = &ast->struct_def;

    ast->range.start = parser_current_location(parser);

    StructKind kind = FRX_STRUCT_KIND_COUNT;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_STRUCT)
    {
        kind = FRX_STRUCT_KIND_NAMED;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT);
    }
    else
    {
        kind = FRX_STRUCT_KIND_UNION;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_UNION);
    }

    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    parser_push_scope(parser);

    AST* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    struct_def_init(struct_def, name, kind, generic_params);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* struct_field = struct_field_parse(parser);
        list_add(&struct_def->fields, struct_field);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_pop_scope(parser);

    ast->range.end = parser_current_location(parser);

    parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_STRUCT,
                         struct_def->name, struct_def);

    return ast;
}

void struct_def_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_DEF);

    ASTStructDef* struct_def = &ast->struct_def;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);
        struct_field_resolve(field, ctx);
    }
}
