#ifndef FRX_TYPE_SYSTEM_H
#define FRX_TYPE_SYSTEM_H

#include "types.h"
#include "token.h"
#include "symbol.h"
#include "list.h"

enum
{
    FRX_TYPE_KIND_PRIMITIVE = 0,
    FRX_TYPE_KIND_FUNC,
    FRX_TYPE_KIND_PTR,
    FRX_TYPE_KIND_ARRAY,
    FRX_TYPE_KIND_SYMBOL,

    FRX_TYPE_KIND_COUNT
};

typedef u8 TypeKind;

typedef struct Type
{
    TypeKind kind;

    union
    {
        struct
        {
            TokenType type;
        } primitive;

        struct
        {
            List params;
            Type* return_type;
            b8 is_variadic;
        } func;

        struct
        {
            struct Type* base;
            b8 mutable;
        } ptr;

        struct
        {
            struct Type* base;
            usize size;
        } array;

        struct
        {
            const Symbol* symbol;
        } symbol;
    };
} Type;

typedef struct TypeInfo
{
    const Type* type;
    List methods;
} TypeInfo;

typedef struct AST AST;

Type* type_create_primitive(TokenType primitive_type);

Type* type_create_func(List* params, AST* return_type, b8 is_variadic);

Type* type_create_ptr(Type* base, b8 mutable);

Type* type_create_array(Type* base, usize size);

Type* type_create_symbol(const Symbol* symbol);

Type* symbol_infer_type(const Symbol* symbol);

void type_system_init(void);

void type_register_method(const Type* type, Symbol* symbol);

Symbol* type_lookup_method(const Type* type, const char* method_name);

List* type_system_get_type_infos(void);

#endif
