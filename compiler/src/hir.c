#include "hir.h"

#include "assert.h"
#include "compiler.h"

static Type* type_create(TypeKind kind)
{
    Type* type = compiler_alloc(sizeof(Type));

    type->kind = kind;

    return type;
}

Type* type_create_primitive(TokenType primitive_type)
{
    FRX_ASSERT(token_type_is_primitive(primitive_type));

    Type* type = type_create(FRX_TYPE_KIND_PRIMITIVE);

    type->primitive_type = primitive_type;

    return type;
}

Type* type_create_symbol(const Symbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    Type* type = type_create(FRX_TYPE_KIND_SYMBOL);

    type->symbol = symbol;

    return type;
}

Type* type_create_ptr(const Type* base_type)
{
    FRX_ASSERT(base_type != NULL);

    Type* type = type_create(FRX_TYPE_KIND_PTR);

    type->base_type = base_type;

    return type;
}

Type* type_create_array(const Type* base_type, usize size)
{
    FRX_ASSERT(base_type != NULL);

    FRX_ASSERT(size > 0);

    Type* type = type_create(FRX_TYPE_KIND_PTR);

    type->base_type = base_type;
    type->size = size;

    return type;
}

Variable* variable_create(const char* name, const Type* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    Variable* variable = compiler_alloc(sizeof(Variable));

    variable->name = name;
    variable->type = type;

    return variable;
}

StructField* struct_field_create(const char* name, const Type* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    StructField* struct_field = compiler_alloc(sizeof(StructField));

    struct_field->name = name;
    struct_field->type = type;

    return struct_field;
}

StructDef* struct_def_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    StructDef* struct_def = compiler_alloc(sizeof(StructDef));

    struct_def->name = name;
    list_init(&struct_def->fields);

    return struct_def;
}

void struct_def_add_field(StructDef* struct_def, StructField* struct_field)
{
    FRX_ASSERT(struct_def != NULL);

    FRX_ASSERT(struct_field != NULL);

    list_add(&struct_def->fields, struct_field);
}

FuncParam* func_param_create(const char* name, const Type* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    FuncParam* func_param = compiler_alloc(sizeof(FuncParam));

    func_param->name = name;
    func_param->type = type;

    return func_param;
};

FuncParams* func_params_create(b8 variadic)
{
    FuncParams* func_params = compiler_alloc(sizeof(FuncParams));

    func_params->variadic = variadic;
    list_init(&func_params->params);

    return func_params;
}

void func_params_add_param(FuncParams* func_params, FuncParam* func_param)
{
    FRX_ASSERT(func_params != NULL);

    FRX_ASSERT(func_param != NULL);

    list_add(&func_params->params, func_param);
}

FuncDef* func_def_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    FuncDef* func_def = compiler_alloc(sizeof(FuncDef));

    func_def->name = name;
    func_def->params = NULL;
    func_def->return_type = NULL;

    return func_def;
}
