#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "symbol_registry.h"
#include "symbol_table.h"
#include "token.h"

static GenericArgs* generic_args_create(void)
{
    GenericArgs* generic_args = compiler_alloc(sizeof(GenericArgs));

    generic_args->error = FRX_FALSE;
    list_init(&generic_args->args);

    return generic_args;
}

static GenericArgs* generic_args_parse(Parser* parser)
{
    GenericArgs* generic_args = generic_args_create();

    generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_LT);

    while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
    {
        if (!list_empty(&generic_args->args))
        {
            generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        GenericArg* arg = compiler_alloc(sizeof(GenericArg));
        arg->type = type_specifier_parse(parser);
        list_add(&generic_args->args, arg);
    }

    generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_GT);

    return generic_args;
}

static TypeSpecifier* type_specifier_create(b8 error, TypeKind kind)
{
    FRX_ASSERT(kind < FRX_TYPE_KIND_COUNT);

    TypeSpecifier* type = compiler_alloc(sizeof(TypeSpecifier));

    type->error = error;
    type->kind = kind;
    type->generic_args = NULL;

    return type;
}

static TypeSpecifier* type_specifier_create_unresolved(b8 error, const char* name)
{
    FRX_ASSERT(name != NULL);

    TypeSpecifier* type = type_specifier_create(error, FRX_TYPE_KIND_UNRESOLVED);

    type->name = name;

    return type;
}

static TypeSpecifier* type_specifier_create_primitive(TokenType primitive)
{
    FRX_ASSERT(token_type_is_primitive(primitive));

    TypeSpecifier* type = type_specifier_create(FRX_FALSE, FRX_TYPE_KIND_PRIMITIVE);

    type->primitive = primitive;

    return type;
}

static TypeSpecifier* type_specifier_create_pointer(TypeSpecifier* base, b8 mutable)
{
    FRX_ASSERT(base != NULL);

    TypeSpecifier* type = type_specifier_create(FRX_FALSE, FRX_TYPE_KIND_POINTER);

    type->ptr.base = base;
    type->ptr.mutable = mutable;

    return type;
}

TypeSpecifier* type_specifier_parse(Parser* parser)
{
    TypeSpecifier* type = NULL;

    if (token_type_is_primitive(parser_current_type(parser)))
    {
        TokenType primitive = parser_current_type(parser);
        parser_eat(parser, primitive);

        type = type_specifier_create_primitive(primitive);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_IDENT)
    {
        const char* name = parser_current_token(parser)->identifier;
        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

        type = type_specifier_create_unresolved(FRX_FALSE, name);
    }
    else
    {
        SourceRange range = parser_current_token(parser)->range;
        FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_TYPE_SPECIFIER,
                                  FRX_DIAGNOSTIC_LVL_ERROR, range, token_type_to_str(parser_current_type(parser)));

        return type_specifier_create_unresolved(FRX_TRUE, "");
    }

    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        type->generic_args = generic_args_parse(parser);
    }

    while (parser_match(parser, FRX_TOKEN_TYPE_STAR) ||
        parser_match(parser, FRX_TOKEN_TYPE_BIT_AND))
    {
        b8 mutable = parser_match(parser, FRX_TOKEN_TYPE_STAR);
        type = type_specifier_create_pointer(type, mutable);

        parser_eat(parser, parser_current_type(parser));
    }

    return type;
}

void type_specifier_resolve(Parser* parser, TypeSpecifier* type)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(type != NULL);

    if (type->error)
    {
        return;
    }

    switch (type->kind)
    {
        case FRX_TYPE_KIND_UNRESOLVED:
        {
            void* symbol = parser_lookup_symbol(parser, FRX_SYMBOL_TYPE_STRUCT, type->name);
            if (symbol != NULL)
            {
                type->kind = FRX_TYPE_KIND_STRUCT;
            }

            break;
        }
        case FRX_TYPE_KIND_PRIMITIVE: break;
        case FRX_TYPE_KIND_ENUM: break;
        case FRX_TYPE_KIND_STRUCT: break;
        case FRX_TYPE_KIND_UNION: break;
        case FRX_TYPE_KIND_POINTER:
        case FRX_TYPE_KIND_ARRAY: type_specifier_resolve(parser, type->ptr.base); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void type_specifier_sema(TypeSpecifier* type)
{
    FRX_ASSERT(type != NULL);

    if (type->error)
    {
        return;
    }
}

void type_specifier_codegen(TypeSpecifier* type)
{
    FRX_ASSERT(type != NULL);
    FRX_ASSERT(!type->error);

    switch (type->kind)
    {
        case FRX_TYPE_KIND_STRUCT: codegen_write("%s", type->name); break;
        case FRX_TYPE_KIND_PRIMITIVE:
        {
            switch (type->primitive)
            {
                case FRX_TOKEN_TYPE_KW_VOID:
                case FRX_TOKEN_TYPE_KW_CHAR:
                {
                    codegen_write("%s", token_type_to_str(type->primitive));
                    break;
                }
                case FRX_TOKEN_TYPE_KW_B8: codegen_write("uint8_t"); break;
                case FRX_TOKEN_TYPE_KW_B16: codegen_write("uint16_t"); break;
                case FRX_TOKEN_TYPE_KW_B32: codegen_write("uint32_t"); break;
                case FRX_TOKEN_TYPE_KW_B64: codegen_write("uint64_t"); break;
                case FRX_TOKEN_TYPE_KW_U8: codegen_write("uint8_t"); break;
                case FRX_TOKEN_TYPE_KW_U16: codegen_write("uint16_t"); break;
                case FRX_TOKEN_TYPE_KW_U32: codegen_write("uint32_t"); break;
                case FRX_TOKEN_TYPE_KW_U64: codegen_write("uint64_t"); break;
                case FRX_TOKEN_TYPE_KW_USIZE: codegen_write("size_t"); break;
                case FRX_TOKEN_TYPE_KW_I8: codegen_write("int8_t"); break;
                case FRX_TOKEN_TYPE_KW_I16: codegen_write("int16_t"); break;
                case FRX_TOKEN_TYPE_KW_I32: codegen_write("int32_t"); break;
                case FRX_TOKEN_TYPE_KW_I64: codegen_write("int_64_t"); break;
                case FRX_TOKEN_TYPE_KW_ISIZE: codegen_write("isize_t"); break;
                case FRX_TOKEN_TYPE_KW_F32: codegen_write("float"); break;
                case FRX_TOKEN_TYPE_KW_F64: codegen_write("double"); break;
                default: FRX_ASSERT(FRX_FALSE);
            }

            break;
        }
        case FRX_TYPE_KIND_POINTER: type_specifier_codegen(type->ptr.base); codegen_write("*"); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}
