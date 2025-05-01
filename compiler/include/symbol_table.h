#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "types.h"

typedef struct Parser Parser;

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
    FRX_SYMBOL_TYPE_EXTERN_FUNC,
    FRX_SYMBOL_TYPE_STRUCT,
    FRX_SYMBOL_TYPE_PARAM,
    FRX_SYMBOL_TYPE_VAR,

    FRX_SYMBOL_TYPE_COUNT
};

typedef u8 SymbolType;

typedef struct Symbol
{
    SymbolVisibility visibility;
    SymbolType type;
    void* data;
} Symbol;

typedef struct SymbolTableEntry
{
    Symbol symbol;
    const char* name;
    struct SymbolTableEntry* next;
} SymbolTableEntry;

#ifndef FRX_SYMBOL_TABLE_CAPACITY
#define FRX_SYMBOL_TABLE_CAPACITY 1024
#endif

typedef struct SymbolTable
{
    SymbolTableEntry* entries[FRX_SYMBOL_TABLE_CAPACITY];
    struct SymbolTable* parent;
} SymbolTable;

void symbol_table_init(SymbolTable* table, SymbolTable* parent);

void symbol_table_insert(SymbolTable* table, SymbolVisibility visibility,
                         SymbolType type, const char* name, void* data);

Symbol* symbol_table_lookup(SymbolTable* table, SymbolType type,
                            const char* name);

#endif
