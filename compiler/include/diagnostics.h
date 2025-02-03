#ifndef FRX_DIAGNOSTICS_H
#define FRX_DIAGNOSTICS_H

#include "types.h"
#include "source_range.h"

#define FRX_DIAGNOSTICS_MAX_ARGS 8

enum
{
    FRX_DIAGNOSTIC_ID_UNEXPECTED_TOKEN,
    FRX_DIAGNOSTIC_ID_EXPECTED_TYPE_SPECIFIER,
    FRX_DIAGNOSTIC_ID_EXPECTED_ITEM,
    FRX_DIAGNOSTIC_ID_EXPECTED_STMT,
    FRX_DIAGNOSTIC_ID_EXPECTED_EXPR,

    FRX_DIAGNOSTIC_ID_COUNT
};

typedef u16 DiagnosticID;

enum
{
    FRX_DIAGNOSTIC_LVL_ERROR,
    FRX_DIAGNOSTIC_LVL_WARNING,
    FRX_DIAGNOSTIC_LVL_NOTE,
    FRX_DIAGNOSTIC_LVL_HELP,

    FRX_DIAGNOSTIC_LVL_COUNT
};

typedef u8 DiagnosticLevel;

typedef struct Diagnostic
{
    DiagnosticID id;
    DiagnosticLevel lvl;
    const char* filepath;
    SourceRange range;
    const char* args[FRX_DIAGNOSTICS_MAX_ARGS];
} Diagnostic;

Diagnostic* diagnostic_create(DiagnosticID id, DiagnosticLevel lvl, const char* filepath, SourceRange range, ...);

void diagnostic_emit(const Diagnostic* d);

#endif
