#ifndef FRX_TYPE_CONTEXT_H
#define FRX_TYPE_CONTEXT_H

#include "ast.h"

enum
{
    FRX_TYPE_KIND_PRIMITIVE,
    FRX_TYPE_KIND_STRUCT,
    FRX_TYPE_KIND_UNION,
    FRX_TYPE_KIND_ENUM,

    FRX_TYPE_KIND_COUNT
};

typedef u8 TypeKind;

typedef struct Type
{
    TypeKind kind;
    const char* name;
} Type;

#endif
