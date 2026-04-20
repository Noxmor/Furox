#ifndef FRX_DIAGNOSTICS_H
#define FRX_DIAGNOSTICS_H

#include "types.h"
#include "source_span.h"

#define FRX_DIAGNOSTICS_MAX_ARGS 8

#define FRX_DIAGNOSTICS(X) \
    X(UNEXPECTED_TOKEN, "Expected '%s', but found '%s'") \
    X(EXPECTED_TYPE_SPECIFIER, "Expected type specifier, but found '%s'") \
    X(EXPECTED_ITEM, "Expected item, but found '%s'") \
    X(EXPECTED_STMT, "Expected statement, but found '%s'") \
    X(EXPECTED_EXPR, "Expected expression, but found '%s'") \
    X(UNRESOLVED_SYMBOL, "Failed to resolve symbol '%s'") \
    X(INVALID_MODULE_PATH, "Invalid module path '%s'")

#define FRX_DIAGNOSTICS_EMIT_NAME(name, format) FRX_DIAGNOSTIC_ID_ ## name,

enum
{
    FRX_DIAGNOSTICS(FRX_DIAGNOSTICS_EMIT_NAME)

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
    SourceSpan span;
    const char* args[FRX_DIAGNOSTICS_MAX_ARGS];
} Diagnostic;

Diagnostic* diagnostic_create(DiagnosticID id, DiagnosticLevel lvl, SourceSpan span, ...);

void diagnostic_emit(const Diagnostic* d);

#endif
