#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static StructField* struct_field_create(const char* name, TypeSpecifier* type)
{
    StructField* field = compiler_alloc_ast(sizeof(StructField));

    field->name = name;
    field->type = type;

    return field;
}

static StructDef* struct_def_create(const char* name, GenericParams* generic_params)
{
    StructDef* struct_def = compiler_alloc_ast(sizeof(StructDef));

    struct_def->name = name;
    struct_def->generic_params = generic_params;
    list_init(&struct_def->fields);

    return struct_def;
}

StructField* struct_field_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    TypeSpecifier* type = type_specifier_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return struct_field_create(name, type);
}

void struct_field_resolve(Parser* parser, StructField* field)
{
    FRX_ASSERT(field != NULL);

    type_specifier_resolve(parser, field->type);
}

StructDef* struct_def_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT);

    const char* name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    GenericParams* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    StructDef* struct_def = struct_def_create(name, generic_params);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        StructField* struct_field = struct_field_parse(parser);
        list_add(&struct_def->fields, struct_field);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_STRUCT,
                         struct_def->name, struct_def);
    return struct_def;
}

static void struct_field_sema(StructField* field)
{
    FRX_ASSERT(field != NULL);

    type_specifier_sema(field->type);
}

void struct_def_resolve(Parser* parser, StructDef* struct_def)
{
    FRX_ASSERT(struct_def != NULL);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);
        struct_field_resolve(parser, field);
    }
}

void struct_def_sema(StructDef* struct_def)
{
    FRX_ASSERT(struct_def != NULL);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);
        struct_field_sema(field);
    }
}

void struct_def_codegen(StructDef* struct_def, CodegenContext* ctx)
{
    FRX_ASSERT(struct_def != NULL);

    FRX_ASSERT(ctx != NULL);

    fprintf(ctx->header, "typedef struct %s %s;\n", struct_def->name, struct_def->name);

    fprintf(ctx->source, "struct %s {\n", struct_def->name);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);
        type_specifier_codegen(field->type, ctx->source);
        fprintf(ctx->source, " %s;\n", field->name);
    }

    fprintf(ctx->source, "};\n");
}
