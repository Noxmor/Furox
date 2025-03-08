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
    FRX_SYMBOL_TYPE_STRUCT,

    FRX_SYMBOL_TYPE_COUNT
};

typedef u8 SymbolType;

typedef struct Symbol
{
    SymbolType type;
    void* data;
} Symbol;

typedef struct SymbolTableEntry
{
    Parser* origin;
    SymbolVisibility visibility;
    const char* name;
    Symbol symbol;
    struct SymbolTableEntry* next;
} SymbolTableEntry;

#ifndef FRX_SYMBOL_TABLE_CAPACITY
#define FRX_SYMBOL_TABLE_CAPACITY 1024
#endif

typedef struct SymbolTable
{
    SymbolTableEntry* entries[FRX_SYMBOL_TABLE_CAPACITY];
} SymbolTable;

void symbol_table_init(SymbolTable* table);

void symbol_table_insert(SymbolTable* table, Parser* origin,
                         SymbolVisibility visibility, SymbolType type,
                         const char* name, void* data);

Symbol* symbol_table_lookup(SymbolTable* table, Parser* origin, const char* name);

#endif
