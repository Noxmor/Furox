#include "assert.h"
#include "ast.h"
#include "hir.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void struct_field_init(ASTStructField* field, const char* name, AST* type)
{
    field->name = name;
    field->type = type;
}

static void struct_def_init(ASTStructDef* struct_def, const char* name,
                            AST* generic_params)
{
    struct_def->name = name;
    struct_def->generic_params = generic_params;
    list_init(&struct_def->fields);
    struct_def->symbol = NULL;
}

AST* struct_field_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_FIELD);
    ASTStructField* field = &ast->struct_field;

    ast->range.start = parser_current_location(parser);

    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* type = type_specifier_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    struct_field_init(field, name, type);

    ast->range.end = parser_current_location(parser);

    return ast;
}

static StructField* struct_field_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_FIELD);

    ASTStructField* field = &ast->struct_field;

    Type* type = type_specifier_resolve(field->type, ctx);

    return struct_field_create(field->name, type);
}

AST* struct_def_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_DEF);
    ASTStructDef* struct_def = &ast->struct_def;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT);

    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    AST* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    struct_def_init(struct_def, name, generic_params);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* struct_field = struct_field_parse(parser);
        list_add(&struct_def->fields, struct_field);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    ast->range.end = parser_current_location(parser);

    struct_def->symbol = parser_insert_symbol(parser, parser->visibility,
                                              FRX_SYMBOL_TYPE_STRUCT,
                                              struct_def->name, NULL);

    return ast;
}

static void struct_field_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_FIELD);

    FRX_ASSERT(ctx != NULL);

    ASTStructField* field = &ast->struct_field;

    type_specifier_sema(field->type, ctx);
}

void struct_def_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_DEF);

    ASTStructDef* struct_def = &ast->struct_def;

    Symbol* symbol = struct_def->symbol;
    StructDef* data = struct_def_create(struct_def->name);
    symbol->data = data;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);
        StructField* struct_field = struct_field_resolve(field, ctx);
        struct_def_add_field(data, struct_field);
    }
}

void struct_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_DEF);

    FRX_ASSERT(ctx != NULL);

    ASTStructDef* struct_def = &ast->struct_def;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);
        struct_field_sema(field, ctx);
    }
}
