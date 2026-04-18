#include "attributes_table.h"

#include <stdlib.h>
#include <string.h>

#define ATTRIBUTES_ENSURE_CAPACITY(attributes, name, id) \
    if (id.local >= attributes->capacity) \
    { \
        usize old_capacity = attributes->capacity; \
        attributes->capacity = attributes->capacity ? attributes->capacity : 1; \
        \
        while (attributes->capacity <= id.local) \
        { \
            attributes->capacity *= 2; \
        } \
        \
        attributes->name = realloc(attributes->name, sizeof(*attributes->name) * attributes->capacity); \
        memset(&attributes->name[old_capacity], 0, (attributes->capacity - old_capacity) * sizeof(*attributes->name)); \
    }

typedef struct TypeAttributes
{
    const Type** types;
    usize capacity;
} TypeAttributes;

typedef struct NameBindingAttributes
{
    ASTNodeID id;
    usize capacity;
} NameBindingAttributes;

typedef struct SourceFileAttributesTable
{
    TypeAttributes type_attributes;
    NameBindingAttributes name_binding_attributes;
} SourceFileAttributesTable;

typedef struct AttributesTable
{
    SourceFileAttributesTable* source_file_attributes;
    usize capacity;
} AttributesTable;

static AttributesTable attributes_table;

static void attributes_table_ensure_capacity(ASTNodeID id)
{
    if (id.owner >= attributes_table.capacity)
    {
        usize old_capacity = attributes_table.capacity;
        attributes_table.capacity = attributes_table.capacity ? attributes_table.capacity : 1;

        while (attributes_table.capacity <= id.owner)
        {
            attributes_table.capacity *= 2;
        }

        attributes_table.source_file_attributes = realloc(attributes_table.source_file_attributes,
                                                sizeof(SourceFileAttributesTable) * attributes_table.capacity);

        memset(&attributes_table.source_file_attributes[old_capacity], 0, (attributes_table.capacity - old_capacity) * sizeof(SourceFileAttributesTable));
    }
}

void attributes_table_insert_type(ASTNodeID id, const Type* type)
{
    attributes_table_ensure_capacity(id);

    SourceFileAttributesTable* table = &attributes_table.source_file_attributes[id.owner];
    TypeAttributes* attributes = &table->type_attributes;
    ATTRIBUTES_ENSURE_CAPACITY(attributes, types, id);

    attributes->types[id.local] = type;
}

const Type* attributes_table_lookup_type(ASTNodeID id)
{
    SourceFileAttributesTable* table = &attributes_table.source_file_attributes[id.owner];

    return table->type_attributes.types[id.local];
}
