#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "types.h"
#include "symbol_registry.h"

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

typedef struct SymbolTableEntry
{
    SymbolType type;
    const char* name;
    SymbolID id;
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

void symbol_table_insert(SymbolTable* table, SymbolType type, const char* name, SymbolID id);

SymbolID symbol_table_lookup(SymbolTable* table, SymbolType type, const char* name);

#endif
