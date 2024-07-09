#include "namespace.h"

#include "core/assert.h"
#include "core/core.h"
#include "core/memory.h"

#include <string.h>

Namespace* namespace_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    Namespace* namespace = memory_alloc(sizeof(Namespace),
            FRX_MEMORY_CATEGORY_UNKNOWN);

    strcpy(namespace->name, name);

    namespace->next = NULL;

    return namespace;
}

void namespace_append(Namespace* namespace, const char* name)
{
    FRX_ASSERT(namespace != NULL);

    FRX_ASSERT(name != NULL);

    while(namespace->next != NULL)
        namespace = namespace->next;

    namespace->next = namespace_create(name);
}

void namespace_drop(Namespace* namespace)
{
    FRX_ASSERT(namespace != NULL);

    FRX_ASSERT(namespace->next != NULL);

    while(namespace->next->next != NULL)
        namespace = namespace->next;

    namespace_delete(namespace->next);

    namespace->next = NULL;
}

Namespace* namespace_duplicate(const Namespace* namespace)
{
    FRX_ASSERT(namespace != NULL);

    Namespace* duplicate = namespace_create(namespace->name);

    if(namespace->next != NULL)
        duplicate->next = namespace_duplicate(namespace->next);

    return duplicate;
}

FRX_NO_DISCARD b8 namespace_equals(const Namespace* left,
        const Namespace* right)
{
    if(left == right)
        return FRX_TRUE;

    if(left == NULL || right == NULL)
        return FRX_FALSE;

    return strcmp(left->name, right->name) == 0 &&
        namespace_equals(left->next, right->next);
}

void namespace_delete(Namespace* namespace)
{
    if(namespace == NULL)
        return;

    namespace_delete(namespace->next);

    memory_free(namespace);
}
