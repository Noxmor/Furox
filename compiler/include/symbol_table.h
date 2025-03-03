#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "types.h"

typedef u64 SymbolID;

SymbolID symbol_intern(const char* name);

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
    SymbolID id;
    Symbol* symbol;
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

Symbol* symbol_table_insert(SymbolTable* table, SymbolID id, SymbolType type, void* data);

void symbol_table_insert_symbol(SymbolTable* table, SymbolID id, Symbol* symbol);

Symbol* symbol_table_lookup(SymbolTable* table, SymbolID id);

#endif
