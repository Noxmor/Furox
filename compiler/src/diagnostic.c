#include "diagnostics.h"

#include <stdio.h>
#include <stdarg.h>

#include "assert.h"
#include "compiler.h"

typedef struct DiagnosticInfo
{
    const char* format;
    usize args_count;
} DiagnosticInfo;

static const DiagnosticInfo diagnostic_id_to_info[FRX_DIAGNOSTIC_ID_COUNT] = {
    [FRX_DIAGNOSTIC_ID_UNEXPECTED_TOKEN] = { "Expected '%s', but found '%s'", 2 },
    [FRX_DIAGNOSTIC_ID_EXPECTED_TYPE_SPECIFIER] = { "Expected type specifier, but found '%s'", 1 },
    [FRX_DIAGNOSTIC_ID_EXPECTED_ITEM] = { "Expected item, but found '%s'", 1 },
    [FRX_DIAGNOSTIC_ID_EXPECTED_STMT] = { "Expected statement, but found '%s'", 1 },
    [FRX_DIAGNOSTIC_ID_EXPECTED_EXPR] = { "Expected expression, but found '%s'", 1 }
};

Diagnostic* diagnostic_create(DiagnosticID id, DiagnosticLevel lvl, const char* filepath,
                              SourceRange range, ...)
{
    FRX_ASSERT(id < FRX_DIAGNOSTIC_ID_COUNT);
    FRX_ASSERT(lvl < FRX_DIAGNOSTIC_LVL_COUNT);
    FRX_ASSERT(filepath != NULL);

    Diagnostic* d = compiler_alloc(sizeof(Diagnostic));

    d->id = id;
    d->lvl = lvl;
    d->filepath = filepath;
    d->range = range;

    usize args_count = diagnostic_id_to_info[d->id].args_count;

    FRX_ASSERT(args_count <= FRX_DIAGNOSTICS_MAX_ARGS);

    if (args_count > 0)
    {
        va_list args;
        va_start(args, range);

        for (usize i = 0; i < args_count; ++i)
        {
            d->args[i] = va_arg(args, const char*);
        }

        va_end(args);
    }

    return d;
}

void diagnostic_emit(const Diagnostic* d)
{
    const char* lvl_str = NULL;
    const char* color_str = NULL;
    FILE* output = stdout;

    switch (d->lvl)
    {
        case FRX_DIAGNOSTIC_LVL_ERROR:
        {
            lvl_str = "ERROR";
            color_str = "\033[1;31m";
            output = stderr;
            break;
        }
        case FRX_DIAGNOSTIC_LVL_WARNING:
        {
            lvl_str = "WARNING";
            color_str = "\033[1;33m";
            output = stderr;
            break;
        }
        case FRX_DIAGNOSTIC_LVL_NOTE:
        {
            lvl_str = "NOTE";
            color_str = "\033[1;36m";
            break;
        }
        case FRX_DIAGNOSTIC_LVL_HELP:
        {
            lvl_str = "HELP";
            color_str = "\033[1;32m";
            break;
        }
        default:
        {
            FRX_ASSERT(FRX_FALSE);
            lvl_str = "";
            color_str = "";
            break;
        }
    }

    const char* clear_color_str = "\033[0m";

    fprintf(output, "[%s%s%s]: %s:%zu:%zu: ", color_str, lvl_str, clear_color_str, d->filepath, d->range.start.line, d->range.start.column);

    const char* format = diagnostic_id_to_info[d->id].format;
    usize args_count = diagnostic_id_to_info[d->id].args_count;

    switch (args_count)
    {
        case 0: fprintf(output, format); break;
        case 1: fprintf(output, format, d->args[0]); break;
        case 2: fprintf(output, format, d->args[0], d->args[1]); break;
        case 3: fprintf(output, format, d->args[0], d->args[1], d->args[2]); break;
        case 4: fprintf(output, format, d->args[0], d->args[1], d->args[2], d->args[3]); break;
        case 5: fprintf(output, format, d->args[0], d->args[1], d->args[2], d->args[3], d->args[4]); break;
        case 6: fprintf(output, format, d->args[0], d->args[1], d->args[2], d->args[3], d->args[4], d->args[5]); break;
        case 7: fprintf(output, format, d->args[0], d->args[1], d->args[2], d->args[3], d->args[4], d->args[5], d->args[6]); break;
        case 8: fprintf(output, format, d->args[0], d->args[1], d->args[2], d->args[3], d->args[4], d->args[5], d->args[6], d->args[7]); break;
        default: FRX_ASSERT(FRX_FALSE);
    }

    fprintf(output, "\n");
}
