#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"

static EnumConstant* enum_constant_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    EnumConstant* constant = compiler_alloc_ast(sizeof(EnumConstant));

    constant->name = name;

    return constant;
}

static EnumConstant* enum_constant_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    return enum_constant_create(name);
}

static EnumDef* enum_def_create(const char* name, TypeSpecifier* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    EnumDef* enum_def = compiler_alloc_ast(sizeof(EnumDef));

    enum_def->name = name;
    enum_def->type = type;
    list_init(&enum_def->constants);

    return enum_def;
}

static void enum_def_add_constant(EnumDef* enum_def, EnumConstant* constant)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(constant != NULL);

    list_add(&enum_def->constants, constant);
}

EnumDef* enum_def_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_ENUM);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    TypeSpecifier* type = type_specifier_parse(parser);

    EnumDef* enum_def = enum_def_create(name, type);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        if (!list_empty(&enum_def->constants))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        EnumConstant* constant = enum_constant_parse(parser);
        enum_def_add_constant(enum_def, constant);
    }

    return enum_def;
}
