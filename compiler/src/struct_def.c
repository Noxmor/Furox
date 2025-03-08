#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static StructField* struct_field_create(b8 error, const char* name, TypeSpecifier* type)
{
    StructField* field = compiler_alloc(sizeof(StructField));

    field->error = error;
    field->name = name;
    field->type = type;

    return field;
}

static StructDef* struct_def_create(b8 error, const char* name)
{
    StructDef* struct_def = compiler_alloc(sizeof(StructDef));

    struct_def->error = error;
    struct_def->name = name;
    list_init(&struct_def->fields);

    return struct_def;
}

StructField* struct_field_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    const char* name = parser_current_token(parser)->identifier;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    error |= parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    TypeSpecifier* type = type_specifier_parse(parser);

    error |= parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return struct_field_create(error, name, type);
}

void struct_field_resolve(Parser* parser, StructField* field)
{
    FRX_ASSERT(field != NULL);

    if (field->error)
    {
        return;
    }

    type_specifier_resolve(parser, field->type);
}

StructDef* struct_def_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error = parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT);

    const char* name = parser_current_token(parser)->identifier;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    error |= parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    StructDef* struct_def = struct_def_create(error, name);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        StructField* struct_field = struct_field_parse(parser);
        list_add(&struct_def->fields, struct_field);
    }

    struct_def->error |= parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_STRUCT,
                         struct_def->name, struct_def);
    return struct_def;
}

static void struct_field_sema(StructField* field)
{
    FRX_ASSERT(field != NULL);

    if (field->error)
    {
        return;
    }

    type_specifier_sema(field->type);
}

void struct_def_resolve(Parser* parser, StructDef* struct_def)
{
    FRX_ASSERT(struct_def != NULL);

    if (struct_def->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);
        struct_field_resolve(parser, field);
    }
}

void struct_def_sema(StructDef* struct_def)
{
    FRX_ASSERT(struct_def != NULL);

    if (struct_def->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);
        struct_field_sema(field);
    }
}

void struct_def_codegen(StructDef* struct_def)
{
    FRX_ASSERT(struct_def != NULL);
    FRX_ASSERT(!struct_def->error);

    codegen_write("typedef struct %s {\n", struct_def->name);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);

        type_specifier_codegen(field->type);
        codegen_write(" %s;\n", field->name);
    }

    codegen_write("} %s;\n", struct_def->name);
}
