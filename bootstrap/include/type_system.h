#ifndef FRX_TYPE_SYSTEM_H
#define FRX_TYPE_SYSTEM_H

#include "types.h"
#include "token.h"
#include "symbol.h"
#include "list.h"

enum
{
    FRX_TYPE_KIND_PRIMITIVE = 0,
    FRX_TYPE_KIND_STRUCT,
    FRX_TYPE_KIND_UNION,
    FRX_TYPE_KIND_ENUM,
    FRX_TYPE_KIND_FUNC,
    FRX_TYPE_KIND_PTR,
    FRX_TYPE_KIND_ARRAY,
    FRX_TYPE_KIND_GENERIC,

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
            const Symbol* symbol;
            const List* generic_args;
        } strct;

        struct
        {
            const Symbol* symbol;
        } enumeration;

        struct
        {
            List params;
            const struct Type* return_type;
            b8 is_variadic;
        } func;

        struct
        {
            const struct Type* base;
            b8 mutable;
        } ptr;

        struct
        {
            const struct Type* base;
            usize size;
        } array;

        struct
        {
            const Symbol* symbol;
        } generic;
    };
} Type;

typedef struct TypeInfo
{
    const Type* type;
    List methods;
} TypeInfo;

void type_system_init(void);

const Type* type_intern_primitive(TokenType primitive);

const Type* type_intern_struct(const Symbol* symbol, const List* generic_args);

const Type* type_intern_enum(const Symbol* symbol);

const Type* type_intern_func(const List* params, const Type* return_type, b8 is_variadic);

const Type* type_intern_ptr(const Type* base, b8 mutable);

const Type* type_intern_array(const Type* base, usize size);

const Type* type_intern_generic(const Symbol* symbol);

const Type* type_intern_char_lit(void);

const Type* type_intern_string_lit(void);

const Type* type_intern_bool(void);

const Type* symbol_infer_type(const Symbol* symbol);

void type_register_method(const Type* type, Symbol* symbol);

Symbol* type_lookup_method(const Type* type, const char* method_name);

List* type_system_get_type_infos(void);

#endif
