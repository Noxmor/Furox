#ifndef FRX_SOURCE_SPAN_H
#define FRX_SOURCE_SPAN_H

#include "types.h"

typedef u32 SourceOffset;
typedef u32 SourceLine;
typedef u32 SourceColumn;

typedef struct SourceSpan
{
    SourceOffset lo;
    SourceOffset hi;
} SourceSpan;

#endif
