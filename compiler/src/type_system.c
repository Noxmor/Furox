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
    FRX_ASSERT(base != NULL);

    Type* type = type_create(FRX_TYPE_KIND_PTR);

    type->ptr.base = base;
    type->ptr.mutable = mutable;

    return type;
}

Type* type_create_array(Type* base, usize size)
{
    FRX_ASSERT(base != NULL);

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
        case FRX_SYMBOL_TYPE_ENUM: return ((ASTEnumDef*)symbol->data)->resolved_type;
        case FRX_SYMBOL_TYPE_ENUM_CONSTANT: return symbol_infer_type(((ASTEnumConstant*)symbol->data)->symbol);
        case FRX_SYMBOL_TYPE_TYPE_ALIAS: return ((ASTTypeAlias*)symbol->data)->type->type_specifier.resolved_type;
        case FRX_SYMBOL_TYPE_PARAM: return ((ASTFuncParam*)symbol->data)->type->type_specifier.resolved_type;
        case FRX_SYMBOL_TYPE_VAR: return ((ASTLetStmt*)symbol->data)->resolved_type;

        default: FRX_ASSERT(FRX_FALSE); return NULL;
    }
}

static List type_infos;

void type_system_init(void)
{
    list_init(&type_infos);
}

void type_register_method(const Type* type, Symbol* symbol)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(symbol != NULL);

    for (usize i = 0; i < list_size(&type_infos); ++i)
    {
        TypeInfo* info = list_get(&type_infos, i);
        if (info->type == type)
        {
            list_add(&info->methods, symbol);
            return;
        }
    }

    TypeInfo* info = compiler_alloc(sizeof(TypeInfo));
    info->type = type;
    list_init(&info->methods);
    list_add(&info->methods, symbol);
    list_add(&type_infos, info);
}

Symbol* type_lookup_method(const Type* type, const char* method_name)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(method_name != NULL);

    while (type->kind == FRX_TYPE_KIND_PTR || type->kind == FRX_TYPE_KIND_ARRAY)
    {
        if (type->kind == FRX_TYPE_KIND_PTR)
        {
            type = type->ptr.base;
        }
        else
        {
            type = type->array.base;
        }
    }

    for (usize i = 0; i < list_size(&type_infos); ++i)
    {
        TypeInfo* info = list_get(&type_infos, i);
        if (info->type != type)
        {
            continue;
        }

        for (usize j = 0; j < list_size(&info->methods); ++j)
        {
            Symbol* method = list_get(&info->methods, j);
            if (method->name == method_name)
            {
                return method;
            }
        }
    }

    return NULL;
}

List* type_system_get_type_infos(void)
{
    return &type_infos;
}
