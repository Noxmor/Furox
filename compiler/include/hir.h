#ifndef FRX_HIR_H
#define FRX_HIR_H

#include "token.h"
#include "list.h"
#include "ast.h"

enum
{
    FRX_TYPE_KIND_PRIMITIVE,
    FRX_TYPE_KIND_SYMBOL,

    FRX_TYPE_KIND_PTR,
    FRX_TYPE_KIND_ARRAY,

    FRX_TYPE_KIND_COUNT
};

typedef u8 TypeKind;

typedef struct Type
{
    TypeKind kind;
    const Symbol* symbol;
    TokenType primitive_type;
    const struct Type* base_type;
    usize size;
} Type;

Type* type_create_primitive(TokenType primitive_type);

Type* type_create_symbol(const Symbol* symbol);

Type* type_create_ptr(const Type* base_type);

Type* type_create_array(const Type* base_type, usize size);

typedef struct Variable
{
    const char* name;
    const Type* type;
} Variable;

Variable* variable_create(const char* name, const Type* type);

typedef struct StructField
{
    const char* name;
    const Type* type;
} StructField;

StructField* struct_field_create(const char* name, const Type* type);

typedef struct StructDef
{
    const char* name;
    List fields;
} StructDef;

StructDef* struct_def_create(const char* name);

void struct_def_add_field(StructDef* struct_def, StructField* struct_field);

typedef struct FuncParam
{
    const char* name;
    const Type* type;
} FuncParam;

FuncParam* func_param_create(const char* name, const Type* type);

typedef struct FuncParams
{
    b8 variadic;
    List params;
} FuncParams;

FuncParams* func_params_create(b8 variadic);

void func_params_add_param(FuncParams* func_params, FuncParam* func_param);

typedef struct FuncDef
{
    const char* name;
    FuncParams* params;
    Type* return_type;
    AST* body;
} FuncDef;

FuncDef* func_def_create(const char* name);

#endif
