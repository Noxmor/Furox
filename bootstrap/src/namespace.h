#ifndef FRX_NAMESPACE_H
#define FRX_NAMESPACE_H

#include "core/core.h"
#include "token.h"

typedef struct Namespace
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    struct Namespace* next;
} Namespace;

Namespace* namespace_create(const char* name);

void namespace_append(Namespace* namespace, const char* name);

void namespace_drop(Namespace* namespace);

Namespace* namespace_duplicate(const Namespace* namespace);

FRX_NO_DISCARD b8 namespace_equals(const Namespace* left,
        const Namespace* right);

void namespace_delete(Namespace* namespace);

#endif
