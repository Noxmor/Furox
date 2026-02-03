#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "symbol.h"

typedef struct SymbolTableEntry
{
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

Symbol* symbol_table_insert(SymbolTable* table, SymbolVisibility visibility,
                            SymbolType type, const char* name, void* data);

Symbol* symbol_table_insert_symbol(SymbolTable* table, Symbol* symbol);

Symbol* symbol_table_lookup(const SymbolTable* table, const char* name);

#endif
