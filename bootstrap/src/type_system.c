#include "assert.h"
#include "attributes_table.h"
#include "compiler.h"
#include "symbol.h"
#include "type_system.h"

static Type* type_create(TypeKind kind)
{
    FRX_ASSERT(kind < FRX_TYPE_KIND_COUNT);

    Type* type = compiler_alloc(sizeof(Type));

    type->kind = kind;

    return type;
}

const Type* type_create_primitive(TokenType primitive_type)
{
    Type* type = type_create(FRX_TYPE_KIND_PRIMITIVE);

    type->primitive.type = primitive_type;

    return type;
}

const Type* type_create_struct(const AST* ast, const List* generic_args)
{
    FRX_ASSERT(ast != NULL);

    const ASTStructDef* struct_def = &ast->struct_def;

    Type* type = type_create(struct_def->kind == FRX_STRUCT_KIND_NAMED ? FRX_TYPE_KIND_STRUCT : FRX_TYPE_KIND_UNION);
    type->strct.ast = ast;
    type->strct.generic_args = generic_args;

    list_add((List*)&struct_def->instantiated_types, type);

    compiler_register_type(type);

    return type;
}

static const Type* type_create_enum(const AST* ast)
{
    FRX_ASSERT(ast != NULL);

    Type* type = type_create(FRX_TYPE_KIND_ENUM);

    type->enumeration.ast = ast;

    return type;
}

const Type* type_create_func(const List* params, const Type* return_type, b8 is_variadic)
{
    FRX_ASSERT(params != NULL);

    FRX_ASSERT(return_type != NULL);

    Type* type = type_create(FRX_TYPE_KIND_FUNC);

    list_init(&type->func.params);

    for (usize i = 0; i < list_size(params); ++i)
    {
        AST* param = list_get(params, i);

        switch (param->type)
        {
            case FRX_AST_TYPE_TYPE_SPECIFIER: list_add(&type->func.params, (Type*)attributes_table_lookup_type(param->id)); break;
            case FRX_AST_TYPE_FUNC_PARAM: list_add(&type->func.params, (Type*)attributes_table_lookup_type(param->func_param.type->id)); break;

            default: FRX_ASSERT(FRX_FALSE); break;
        }
    }

    type->func.return_type = return_type;
    type->func.is_variadic = is_variadic;

    return type;
}

const Type* type_create_ptr(const Type* base, b8 mutable)
{
    FRX_ASSERT(base != NULL);

    Type* type = type_create(FRX_TYPE_KIND_PTR);

    type->ptr.base = base;
    type->ptr.mutable = mutable;

    return type;
}

const Type* type_create_array(const Type* base, AST* size)
{
    FRX_ASSERT(base != NULL);

    Type* type = type_create(FRX_TYPE_KIND_ARRAY);

    type->array.base = base;
    type->array.size = size;

    return type;
}

static const Type* type_create_generic(const Symbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    Type* type = type_create(FRX_TYPE_KIND_GENERIC);

    type->generic.symbol = symbol;

    return type;
}

#define FRX_TYPE_TABLE_CAPACITY 1024

typedef struct TypeTableEntry
{
    const Type* type;
    struct TypeTableEntry* next;
} TypeTableEntry;

static TypeTableEntry* type_table_entry_create(const Type* type, TypeTableEntry* next)
{
    FRX_ASSERT(type != NULL);

    TypeTableEntry* entry = compiler_alloc(sizeof(TypeTableEntry));
    entry->type = type;
    entry->next = next;

    return entry;
}

typedef struct TypeTable
{
    TypeTableEntry* entries[FRX_TYPE_TABLE_CAPACITY];
} TypeTable;

static TypeTable type_table;

static const Type u8_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_U8
};

static const Type u16_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_U16
};

static const Type u32_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_U32
};

static const Type u64_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_U64
};

static const Type usize_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_USIZE
};

static const Type i8_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_I8
};

static const Type i16_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_I16
};

static const Type i32_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_I32
};

static const Type i64_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_I64
};

static const Type isize_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_ISIZE
};

static const Type b8_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_B8
};

static const Type b16_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_B16
};

static const Type b32_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_B32
};

static const Type b64_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_B64
};

static const Type char_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_CHAR
};

static const Type f32_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_F32
};
static const Type f64_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_F64
};
static const Type void_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive.type = FRX_TOKEN_TYPE_KW_VOID
};

static const Type string_literal_type = {
    .kind = FRX_TYPE_KIND_PTR,
    .ptr = {
        .base = &char_type,
        .mutable = FRX_FALSE
    }
};

static const Type nullptr_type = {
    .kind = FRX_TYPE_KIND_PTR,
    .ptr = {
        .base = &void_type,
        .mutable = FRX_TRUE
    }
};

const Type* type_intern_primitive(TokenType primitive)
{
    switch (primitive)
    {
        case FRX_TOKEN_TYPE_KW_U8: return &u8_type;
        case FRX_TOKEN_TYPE_KW_U16: return &u16_type;
        case FRX_TOKEN_TYPE_KW_U32: return &u32_type;
        case FRX_TOKEN_TYPE_KW_U64: return &u64_type;
        case FRX_TOKEN_TYPE_KW_USIZE: return &usize_type;
        case FRX_TOKEN_TYPE_KW_I8: return &i8_type;
        case FRX_TOKEN_TYPE_KW_I16: return &i16_type;
        case FRX_TOKEN_TYPE_KW_I32: return &i32_type;
        case FRX_TOKEN_TYPE_KW_I64: return &i64_type;
        case FRX_TOKEN_TYPE_KW_ISIZE: return &isize_type;
        case FRX_TOKEN_TYPE_KW_B8: return &b8_type;
        case FRX_TOKEN_TYPE_KW_B16: return &b16_type;
        case FRX_TOKEN_TYPE_KW_B32: return &b32_type;
        case FRX_TOKEN_TYPE_KW_B64: return &b64_type;
        case FRX_TOKEN_TYPE_KW_BOOL: return &b8_type;
        case FRX_TOKEN_TYPE_KW_CHAR: return &char_type;
        case FRX_TOKEN_TYPE_KW_F32: return &f32_type;
        case FRX_TOKEN_TYPE_KW_F64: return &f64_type;
        case FRX_TOKEN_TYPE_KW_VOID: return &void_type;

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return NULL;
}

const Type* type_intern_struct(const AST* ast, const List* generic_args)
{
    FRX_ASSERT(ast != NULL);

    u64 index =  (usize)ast % FRX_TYPE_TABLE_CAPACITY;
    TypeTableEntry* entry = type_table.entries[index];
    while (entry != NULL)
    {
        const Type* type = entry->type;
        if ((type->kind == FRX_TYPE_KIND_STRUCT || type->kind == FRX_TYPE_KIND_UNION) && type->strct.ast == ast)
        {
            return type;
        }

        entry = entry->next;
    }

    const Type* type = type_create_struct(ast, generic_args);
    type_table.entries[index] = type_table_entry_create(type, type_table.entries[index]);

    return type;
}

const Type* type_intern_enum(const AST* ast)
{
    FRX_ASSERT(ast != NULL);

    u64 index =  (usize)ast % FRX_TYPE_TABLE_CAPACITY;
    TypeTableEntry* entry = type_table.entries[index];
    while (entry != NULL)
    {
        const Type* type = entry->type;
        if (type->kind == FRX_TYPE_KIND_ENUM && type->enumeration.ast == ast)
        {
            return type;
        }

        entry = entry->next;
    }

    const Type* type = type_create_enum(ast);
    type_table.entries[index] = type_table_entry_create(type, type_table.entries[index]);

    return type;
}

const Type* type_intern_func(const List* params, const Type* return_type, b8 is_variadic)
{
    u64 index =  (usize)return_type % FRX_TYPE_TABLE_CAPACITY;
    TypeTableEntry* entry = type_table.entries[index];
    while (entry != NULL)
    {
        const Type* type = entry->type;
        if (type->kind == FRX_TYPE_KIND_FUNC
            && list_size(&type->func.params) == list_size(params)
            && type->func.return_type == return_type
            &&type->func.is_variadic == is_variadic)
        {
            b8 equal = FRX_TRUE;
            for (usize i = 0; i < list_size(&type->func.params); ++i)
            {
                const Type* param_type = list_get(&type->func.params, i);
                const AST* param = list_get(params, i);

                switch (param->type)
                {
                    case FRX_AST_TYPE_TYPE_SPECIFIER: equal = param_type == attributes_table_lookup_type(param->id); break;
                    case FRX_AST_TYPE_FUNC_PARAM: equal = param_type == attributes_table_lookup_type(param->func_param.type->id); break;

                    default: FRX_ASSERT(FRX_FALSE); break;
                }

                if (!equal)
                {
                    break;
                }
            }

            if (equal)
            {
                return type;
            }
        }

        entry = entry->next;
    }

    const Type* type = type_create_func(params, return_type, is_variadic);
    type_table.entries[index] = type_table_entry_create(type, type_table.entries[index]);

    return type;
}

const Type* type_intern_ptr(const Type* base, b8 mutable)
{
    u64 index =  (usize)base % FRX_TYPE_TABLE_CAPACITY;
    TypeTableEntry* entry = type_table.entries[index];
    while (entry != NULL)
    {
        const Type* type = entry->type;
        if (type->kind == FRX_TYPE_KIND_PTR && type->ptr.base == base && type->ptr.mutable == mutable)
        {
            return type;
        }

        entry = entry->next;
    }

    const Type* type = type_create_ptr(base, mutable);
    type_table.entries[index] = type_table_entry_create(type, type_table.entries[index]);

    return type;
}

const Type* type_intern_array(const Type* base, AST* size)
{
    u64 index =  (usize)base % FRX_TYPE_TABLE_CAPACITY;
    TypeTableEntry* entry = type_table.entries[index];
    while (entry != NULL)
    {
        const Type* type = entry->type;
        if (type->kind == FRX_TYPE_KIND_ARRAY && type->array.base == base
            && type->array.size == size)
        {
            return type;
        }

        entry = entry->next;
    }

    const Type* type = type_create_array(base, size);
    type_table.entries[index] = type_table_entry_create(type, type_table.entries[index]);

    return type;
}

const Type* type_intern_generic(const Symbol* symbol)
{
    u64 index =  (usize)symbol % FRX_TYPE_TABLE_CAPACITY;
    TypeTableEntry* entry = type_table.entries[index];
    while (entry != NULL)
    {
        const Type* type = entry->type;
        if (type->kind == FRX_TYPE_KIND_GENERIC && type->generic.symbol == symbol)
        {
            return type;
        }

        entry = entry->next;
    }

    const Type* type = type_create_generic(symbol);
    type_table.entries[index] = type_table_entry_create(type, type_table.entries[index]);

    return type;
}

const Type* type_intern_char_lit(void)
{
    return &char_type;
}

const Type* type_intern_string_lit(void)
{
    return &string_literal_type;
}

const Type* type_intern_bool(void)
{
    return &b8_type;
}

const Type* type_intern_nullptr(void)
{
    return &nullptr_type;
}

const Type* symbol_infer_type(const Symbol* symbol)
{
    FRX_ASSERT(symbol != NULL);

    switch (symbol->type)
    {
        case FRX_SYMBOL_TYPE_PRIMITIVE: return type_intern_primitive((symbol->data)->path_segment.primitive);
        case FRX_SYMBOL_TYPE_FUNC: return attributes_table_lookup_type((symbol->data)->id);
        case FRX_SYMBOL_TYPE_STRUCT: return type_intern_struct(symbol->data, NULL);
        case FRX_SYMBOL_TYPE_ENUM: return type_intern_enum(symbol->data);
        case FRX_SYMBOL_TYPE_ENUM_VARIANT: return symbol_infer_type((symbol->data)->enum_variant.symbol);
        case FRX_SYMBOL_TYPE_TYPE_ALIAS: return attributes_table_lookup_type((symbol->data)->type_alias.type->id);
        case FRX_SYMBOL_TYPE_PARAM: return attributes_table_lookup_type((symbol->data)->func_param.type->id);
        case FRX_SYMBOL_TYPE_VAR: return attributes_table_lookup_type(symbol->data->id);
        case FRX_SYMBOL_TYPE_GENERIC_PARAM: return type_intern_generic(symbol);
        case FRX_SYMBOL_TYPE_STATIC: return attributes_table_lookup_type(symbol->data->id);

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
