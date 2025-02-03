#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "types.h"

typedef u64 SymbolID;

typedef struct Symbol
{
    const char* name;
    SymbolID id;
} Symbol;

Symbol* symbol_intern(const char* name);

void symbol_table_shutdown(void);

#endif
