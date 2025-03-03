#ifndef FRX_PROJECT_SPECIFICATION_H
#define FRX_PROJECT_SPECIFICATION_H

#include "types.h"

enum
{
    FRX_PROJECT_TYPE_APP,
    FRX_PROJECT_TYPE_LIB,

    FRX_PROJECT_TYPE_COUNT
};

typedef u8 ProjectType;

typedef struct ProjectSpecificiation
{
    ProjectType type;
} ProjectSpecificiation;

#endif
