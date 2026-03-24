#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "symbol.h"

static void let_stmt_init(ASTLetStmt* let_stmt, b8 mutable, const char* name,
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
    ASTLetStmt* let_stmt = &ast->let_stmt;

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

    parser_insert_symbol(parser, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_VAR,
                         name, let_stmt);

    return ast;
}

void let_stmt_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    ASTLetStmt* let_stmt = &ast->let_stmt;

    if (let_stmt->value != NULL)
    {
        ast_resolve(let_stmt->value, ctx);
    }

    if (let_stmt->type != NULL)
    {
        type_specifier_resolve(let_stmt->type, ctx);
        let_stmt->resolved_type = let_stmt->type->type_specifier.resolved_type;
    }
    else if (let_stmt->value != NULL)
    {
        let_stmt->resolved_type = expr_infer_type(let_stmt->value);
    }
}

void let_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_LET_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTLetStmt* let_stmt = &ast->let_stmt;

    if (let_stmt->value != NULL)
    {
        ast_sema(let_stmt->value, ctx);
    }
}
