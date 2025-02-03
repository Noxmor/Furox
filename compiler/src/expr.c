#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

typedef void (*ExprSemaFunc)(void*);

static const ExprSemaFunc expr_type_to_sema[FRX_EXPR_TYPE_COUNT] = {
    [FRX_EXPR_TYPE_INT_LIT] = (ExprSemaFunc)int_literal_sema
};

typedef void (*ExprCodegenFunc)(void*);

static const ExprCodegenFunc expr_type_to_codegen[FRX_EXPR_TYPE_COUNT] = {
    [FRX_EXPR_TYPE_INT_LIT] = (ExprCodegenFunc)int_literal_codegen,
};
static Expr* expr_create(ExprType type, void* node)
{
    FRX_ASSERT(type < FRX_EXPR_TYPE_COUNT);
    FRX_ASSERT(node != NULL);

    Expr* expr = compiler_alloc(sizeof(Expr));

    expr->type = type;
    expr->node = node;

    return expr;
}

Expr* expr_parse(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_INT_LIT: return expr_create(FRX_EXPR_TYPE_INT_LIT, int_literal_parse(parser));
        default:
        {
            FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_EXPR,
                                      FRX_DIAGNOSTIC_LVL_ERROR,
                                      parser_current_token(parser)->range,
                                      token_type_to_str(parser_current_type(parser)));
            parser_recover(parser);

            return NULL;
        }
    }
}

void expr_sema(Expr* expr)
{
    FRX_ASSERT(expr != NULL);

    expr_type_to_sema[expr->type](expr->node);
}

void expr_codegen(Expr* expr)
{
    FRX_ASSERT(expr != NULL);

    expr_type_to_codegen[expr->type](expr->node);
}
