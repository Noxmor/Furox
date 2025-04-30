#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "type_inference.h"

static void let_stmt_init(LetStmt* let_stmt, b8 mutable, const char* name,
                              AST* type, AST* value)
{
    FRX_ASSERT(name != NULL);

    let_stmt->mutable = mutable;
    let_stmt->name = name;
    let_stmt->type = type;
    let_stmt->value = value;
}

AST* let_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_LET_STMT);
    LetStmt* let_stmt = &ast->let_stmt;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_LET);

    b8 mutable = FRX_FALSE;
    if (parser_match(parser, FRX_TOKEN_TYPE_KW_MUT))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_MUT);
        mutable = FRX_TRUE;
    }

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    AST* type = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_COLON))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_COLON);

        type = type_specifier_parse(parser);
    }

    AST* value = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_EQ))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_EQ);
        value = expr_parse(parser);
    }

    if (type == NULL && value == NULL)
    {
        //TODO: Error: cannot have variable declaration without explicit type
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    let_stmt_init(let_stmt, mutable, name, type, value);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void let_stmt_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    LetStmt* let_stmt = &ast->let_stmt;

    if (let_stmt->type != NULL)
    {
        type_specifier_resolve(let_stmt->type, parser);
    }

    if (let_stmt->value != NULL)
    {
        ast_resolve(let_stmt->value, parser);
    }
}

void let_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    FRX_ASSERT(ctx != NULL);

    LetStmt* let_stmt = &ast->let_stmt;

    if (let_stmt->type != NULL)
    {
        type_specifier_sema(let_stmt->type, ctx);
    }

    if (let_stmt->value != NULL)
    {
        ast_sema(let_stmt->value, ctx);

        if (let_stmt->type == NULL)
        {
            AST* type = ast_create(FRX_AST_TYPE_TYPE_SPECIFIER);
            type->type_specifier = *expr_infer_type(let_stmt->value);
            let_stmt->type = type;
        }
    }
}

void let_stmt_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    FRX_ASSERT(ctx != NULL);

    LetStmt* let_stmt = &ast->let_stmt;

    type_specifier_codegen(let_stmt->type, ctx->source);
    fprintf(ctx->source, " %s", let_stmt->name);

    if (let_stmt->value != NULL)
    {
        fprintf(ctx->source, " = ");
        ast_codegen(let_stmt->value, ctx);
    }

    fprintf(ctx->source, ";\n");
}
