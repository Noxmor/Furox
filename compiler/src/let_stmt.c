#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "type_inference.h"
#include "codegen.h"

static LetStmt* let_stmt_create(const char* name, TypeSpecifier* type, Expr* value)
{
    FRX_ASSERT(name != NULL);

    LetStmt* let_stmt = compiler_alloc(sizeof(LetStmt));

    let_stmt->name = name;
    let_stmt->type = type;
    let_stmt->value = value;

    return let_stmt;
}

LetStmt* let_stmt_parse(Parser* parser)
{
    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_LET))
    {
        return NULL;
    }

    const char* name = parser_current_token(parser)->identifier;
    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        return NULL;
    }

    TypeSpecifier* type = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_COLON))
    {
        if (parser_eat(parser, FRX_TOKEN_TYPE_COLON))
        {
            return NULL;
        }

        type = type_specifier_parse(parser);
    }

    Expr* value = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_EQ))
    {
        if (parser_eat(parser, FRX_TOKEN_TYPE_EQ))
        {
            return NULL;
        }

        value = expr_parse(parser);
    }

    if (type == NULL && value == NULL)
    {
        //TODO: Error: cannot have variable declaration without explicit type
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_SEMI))
    {
        return NULL;
    }

    return let_stmt_create(name, type, value);
}

void let_stmt_resolve(Parser* parser, LetStmt* let_stmt)
{
    FRX_ASSERT(let_stmt != NULL);

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

void let_stmt_codegen(LetStmt* let_stmt)
{
    FRX_ASSERT(let_stmt != NULL);

    type_specifier_codegen(let_stmt->type);
    codegen_write(" %s", let_stmt->name);

    if (let_stmt->value != NULL)
    {
        codegen_write(" = ");
        expr_codegen(let_stmt->value);
    }

    codegen_write(";\n");
}
