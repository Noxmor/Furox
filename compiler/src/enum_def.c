#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static AST* enum_constant_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_ENUM_CONSTANT);
    EnumConstant* constant = &ast->enum_constant;

    constant->name = name;

    return ast;
}

static AST* enum_constant_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    return enum_constant_create(name);
}

static void enum_def_init(EnumDef* enum_def, const char* name, AST* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    enum_def->name = name;
    enum_def->type = type;
    list_init(&enum_def->constants);
}

static void enum_def_add_constant(EnumDef* enum_def, AST* constant)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(constant != NULL);

    FRX_ASSERT(constant->type == FRX_AST_TYPE_ENUM_CONSTANT);

    list_add(&enum_def->constants, constant);
}

AST* enum_def_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_ENUM_DEF);
    EnumDef* enum_def = &ast->enum_def;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_ENUM);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* type = type_specifier_parse(parser);

    enum_def_init(enum_def, name, type);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        if (!list_empty(&enum_def->constants))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        AST* constant = enum_constant_parse(parser);
        enum_def_add_constant(enum_def, constant);
    }

    ast->range.end = parser_current_location(parser);

    return ast;
}

void enum_def_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    // TODO: Implement
    (void)parser;
}

void enum_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    FRX_ASSERT(ctx != NULL);

    // TODO: Implement
}

void enum_def_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    FRX_ASSERT(ctx != NULL);

    // TODO: Implement
}
