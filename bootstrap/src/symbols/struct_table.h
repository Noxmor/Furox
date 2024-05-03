#ifndef FRX_STRUCT_TABLE_H
#define FRX_STRUCT_TABLE_H

#include "containers/list.h"

#include "namespace.h"
#include "token.h"

#define FRX_STRUCT_TABLE_CAPACITY 1024

typedef struct StructSymbol
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    List fields;

    Namespace* namespace;

    b8 defined;
} StructSymbol;

typedef struct StructTableEntry
{
    StructSymbol symbol;

    struct StructTableEntry* next;
} StructTableEntry;

typedef struct StructTable
{
    StructTableEntry* entries[FRX_STRUCT_TABLE_CAPACITY];
} StructTable;

void struct_table_init(StructTable* table);

StructSymbol* struct_table_insert(StructTable* table, const char* name,
        const Namespace* namespace);

StructSymbol* struct_table_find(const StructTable* table, const char* name,
        const Namespace* namespace);

StructSymbol* struct_table_find_or_insert(StructTable* table, const char* name,
        const Namespace* namespace);

void struct_table_delete(StructTable* table);

#endif
