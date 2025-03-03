#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static StructField* struct_field_create(const char* name, TypeSpecifier* type)
{
    StructField* field = compiler_alloc(sizeof(StructField));

    field->name = name;
    field->type = type;

    return field;
}

static StructDef* struct_def_create(const char* name)
{
    StructDef* struct_def = compiler_alloc(sizeof(StructDef));

    struct_def->id = symbol_intern(name);
    struct_def->name = name;
    list_init(&struct_def->fields);

    return struct_def;
}

StructField* struct_field_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;

    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        return NULL;
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_COLON))
    {
        return NULL;
    }

    TypeSpecifier* type = type_specifier_parse(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_SEMI))
    {
        return NULL;
    }

    return struct_field_create(name, type);
}
StructDef* struct_def_parse(Parser* parser)
{
    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT))
    {
        return NULL;
    }

    const char* name = parser_current_token(parser)->identifier;

    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        return NULL;
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_LBRACE))
    {
        return NULL;
    }

    StructDef* struct_def = struct_def_create(name);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        StructField* struct_field = struct_field_parse(parser);
        if (struct_field != NULL)
        {
            list_add(&struct_def->fields, struct_field);
        }
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        return NULL;
    }

    parser_insert_symbol(parser, FRX_SYMBOL_VISIBILITY_PRIVATE, struct_def->id,
                         FRX_SYMBOL_TYPE_STRUCT, struct_def);
    return struct_def;
}

static void struct_field_sema(StructField* field)
{
    FRX_ASSERT(field != NULL);

    type_specifier_sema(field->type);
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

void struct_def_codegen(StructDef* struct_def)
{
    FRX_ASSERT(struct_def != NULL);

    codegen_write("typedef struct %s {\n", struct_def->name);

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        StructField* field = list_get(&struct_def->fields, i);

        type_specifier_codegen(field->type);
        codegen_write(" %s;\n", field->name);
    }

    codegen_write("} %s;\n", struct_def->name);
}
