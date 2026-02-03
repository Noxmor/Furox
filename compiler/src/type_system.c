#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "type_system.h"

static Type* type_create(TypeKind kind)
{
    FRX_ASSERT(kind < FRX_TYPE_KIND_COUNT);

    Type* type = compiler_alloc(sizeof(Type));

    type->kind = kind;

    return type;
}

Type* type_create_primitive(TokenType primitive_type)
{
    Type* type = type_create(FRX_TYPE_KIND_PRIMITIVE);

    type->primitive.type = primitive_type;

    return type;
}

Type* type_create_ptr(Type* base, b8 mutable)
{
    Type* type = type_create(FRX_TYPE_KIND_PTR);

    type->ptr.base = base;
    type->ptr.mutable = mutable;

    return type;
}

Type* type_create_array(Type* base, usize size)
{
    Type* type = type_create(FRX_TYPE_KIND_ARRAY);

    type->array.base = base;
    type->array.size = size;

    return type;
}

Type* type_create_symbol(const Symbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    Type* type = type_create(FRX_TYPE_KIND_SYMBOL);

    type->symbol.symbol = symbol;

    return type;
}

Type* symbol_infer_type(const Symbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    switch (symbol->type)
    {
        case FRX_SYMBOL_TYPE_FUNC: return ((ASTFuncDecl*)symbol->data)->return_type->type_specifier.resolved_type;
        case FRX_SYMBOL_TYPE_STRUCT: return ((ASTStructDef*)symbol->data)->resolved_type;
        case FRX_SYMBOL_TYPE_UNION: return NULL; // TODO: Implement
        case FRX_SYMBOL_TYPE_ENUM: return ((ASTEnumDef*)symbol->data)->resolved_type;
        case FRX_SYMBOL_TYPE_PARAM: return ((ASTFuncParam*)symbol->data)->type->type_specifier.resolved_type;
        case FRX_SYMBOL_TYPE_VAR: return ((ASTLetStmt*)symbol->data)->resolved_type;

        default: FRX_ASSERT(FRX_FALSE); return NULL;
    }
}
