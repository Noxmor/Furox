#ifndef FRX_SOURCE_RANGE_H
#define FRX_SOURCE_RANGE_H

#include "source_location.h"

typedef struct SourceRange
{
    SourceLocation start;
    SourceLocation end;
} SourceRange;

#endif
