#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void struct_field_init(StructField* field, const char* name, AST* type)
{
    field->name = name;
    field->type = type;
}

static void struct_def_init(StructDef* struct_def, const char* name,
                            AST* generic_params)
{
    struct_def->name = name;
    struct_def->generic_params = generic_params;
    list_init(&struct_def->fields);
}

AST* struct_field_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_FIELD);
    StructField* field = &ast->struct_field;

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

void struct_field_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_FIELD);

    StructField* field = &ast->struct_field;

    type_specifier_resolve(field->type, parser);
}

AST* struct_def_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_STRUCT_DEF);
    StructDef* struct_def = &ast->struct_def;

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

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_STRUCT,
                         struct_def->name, struct_def);

    ast->range.end = parser_current_location(parser);

    return ast;
}

static void struct_field_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_FIELD);

    FRX_ASSERT(ctx != NULL);

    StructField* field = &ast->struct_field;

    type_specifier_sema(field->type, ctx);
}

void struct_def_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_DEF);

    StructDef* struct_def = &ast->struct_def;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);
        struct_field_resolve(field, parser);
    }
}

void struct_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_DEF);

    FRX_ASSERT(ctx != NULL);

    StructDef* struct_def = &ast->struct_def;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);
        struct_field_sema(field, ctx);
    }
}

void struct_def_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_STRUCT_DEF);

    FRX_ASSERT(ctx != NULL);

    StructDef* struct_def = &ast->struct_def;

    fprintf(ctx->header, "typedef struct %s %s;\n", struct_def->name, struct_def->name);

    fprintf(ctx->source, "struct %s {\n", struct_def->name);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* struct_field = list_get(&struct_def->fields, i);
        StructField* field = &struct_field->struct_field;
        type_specifier_codegen(field->type, ctx->source);
        fprintf(ctx->source, " %s;\n", field->name);
    }

    fprintf(ctx->source, "};\n");
}
