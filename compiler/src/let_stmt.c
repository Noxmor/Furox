#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "type_inference.h"

static LetStmt* let_stmt_create(b8 error, const char* name, TypeSpecifier* type, Expr* value)
{
    FRX_ASSERT(name != NULL);

    LetStmt* let_stmt = compiler_alloc_ast(sizeof(LetStmt));

    let_stmt->error = error;
    let_stmt->name = name;
    let_stmt->type = type;
    let_stmt->value = value;

    return let_stmt;
}

LetStmt* let_stmt_parse(Parser* parser)
{
    b8 error = FRX_FALSE;
    error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_LET);

    const char* name = parser_current_token(parser)->identifier;
    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    TypeSpecifier* type = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_COLON))
    {
        error |= parser_eat(parser, FRX_TOKEN_TYPE_COLON);

        type = type_specifier_parse(parser);
    }

    Expr* value = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_EQ))
    {
        error |= parser_eat(parser, FRX_TOKEN_TYPE_EQ);
        value = expr_parse(parser);
    }

    if (type == NULL && value == NULL)
    {
        //TODO: Error: cannot have variable declaration without explicit type
    }

    error |= parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return let_stmt_create(error, name, type, value);
}

void let_stmt_resolve(Parser* parser, LetStmt* let_stmt)
{
    FRX_ASSERT(let_stmt != NULL);

    if (let_stmt->error)
    {
        return;
    }

    if (let_stmt->type != NULL)
    {
        type_specifier_resolve(parser, let_stmt->type);
    }

    if (let_stmt->value != NULL)
    {
        expr_resolve(parser, let_stmt->value);
    }
}

void let_stmt_sema(LetStmt* let_stmt)
{
    FRX_ASSERT(let_stmt != NULL);

    if (let_stmt->error)
    {
        return;
    }

    if (let_stmt->type != NULL)
    {
        type_specifier_sema(let_stmt->type);
    }

    if (let_stmt->value != NULL)
    {
        expr_sema(let_stmt->value);

        if (let_stmt->type == NULL)
        {
            let_stmt->type = expr_infer_type(let_stmt->value);
        }
    }
}
