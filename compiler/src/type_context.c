#include "type_context.h"

void type_context_init(TypeContext* ctx)
{
    FRX_ASSERT(ctx != NULL);
}

Type* type_context_resolve(AST* ast)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    // TODO: Implement

    return NULL;
}
