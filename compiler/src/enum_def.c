#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static AST* enum_constant_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_ENUM_CONSTANT);
    ASTEnumConstant* constant = &ast->enum_constant;

    constant->name = name;

    return ast;
}

static AST* enum_constant_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    return enum_constant_create(name);
}

static void enum_def_init(ASTEnumDef* enum_def, const char* name, AST* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    enum_def->name = name;
    enum_def->type = type;
    list_init(&enum_def->constants);
    enum_def->resolved_type = NULL;
}

static void enum_def_add_constant(ASTEnumDef* enum_def, AST* constant)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(constant != NULL);

    FRX_ASSERT(constant->type == FRX_AST_TYPE_ENUM_CONSTANT);

    list_add(&enum_def->constants, constant);
}

AST* enum_def_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_ENUM_DEF);
    ASTEnumDef* enum_def = &ast->enum_def;

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

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_ENUM,
                         name, enum_def);

    return ast;
}

void enum_def_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    // TODO: Implement
    (void)ast;
    (void)ctx;
}

void enum_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    FRX_ASSERT(ctx != NULL);

    // TODO: Implement
    (void)ast;
    (void)ctx;
}
