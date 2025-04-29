#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "parser.h"

static IntLiteral* int_literal_create(u64 value)
{
    IntLiteral* literal = compiler_alloc_ast(sizeof(IntLiteral));

    literal->value = value;

    return literal;
}

IntLiteral* int_literal_parse(Parser* parser)
{
    u64 value = parser_current_token(parser)->int_literal;

    parser_eat(parser, FRX_TOKEN_TYPE_INT_LIT);

    return int_literal_create(value);
}

void int_literal_sema(IntLiteral* literal)
{
    FRX_ASSERT(literal != NULL);
}

void int_literal_codegen(IntLiteral* literal, CodegenContext* ctx)
{
    FRX_ASSERT(literal != NULL);

    FRX_ASSERT(ctx != NULL);

    fprintf(ctx->source, "%zu", literal->value);
}
