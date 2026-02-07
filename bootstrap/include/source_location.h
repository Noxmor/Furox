#ifndef FRX_SOURCE_LOCATION_H
#define FRX_SOURCE_LOCATION_H

#include "types.h"

typedef struct SourceLocation
{
    usize pos;
    usize line;
    usize column;
} SourceLocation;

#endif
