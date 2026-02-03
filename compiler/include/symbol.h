#ifndef FRX_SYMBOL_H
#define FRX_SYMBOL_H

#include "types.h"

enum
{
    FRX_SYMBOL_VISIBILITY_PRIVATE,
    FRX_SYMBOL_VISIBILITY_MODULE,
    FRX_SYMBOL_VISIBILITY_PUBLIC,

    FRX_SYMBOL_VISIBILITY_COUNT
};

typedef u8 SymbolVisibility;

enum
{
    FRX_SYMBOL_TYPE_FUNC,
    FRX_SYMBOL_TYPE_STRUCT,
    FRX_SYMBOL_TYPE_UNION,
    FRX_SYMBOL_TYPE_ENUM,
    FRX_SYMBOL_TYPE_PARAM,
    FRX_SYMBOL_TYPE_VAR,

    FRX_SYMBOL_TYPE_COUNT
};

typedef u8 SymbolType;

typedef struct Symbol
{
    const char* name;
    SymbolVisibility visibility;
    SymbolType type;
    void* data;
} Symbol;

Symbol* symbol_create(const char* name, SymbolVisibility visibility,
                      SymbolType type, void* data);

#endif
