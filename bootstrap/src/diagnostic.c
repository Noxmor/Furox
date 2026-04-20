#include "diagnostics.h"

#include <stdio.h>
#include <stdarg.h>

#include "assert.h"
#include "compiler.h"
#include "source_map.h"

#define FRX_DIAGNOSTICS_EMIT_FORMAT(name, format) format,

static const char* diagnostic_id_to_format[] = {
    FRX_DIAGNOSTICS(FRX_DIAGNOSTICS_EMIT_FORMAT)
};

static usize format_args_count(const char* format)
{
    FRX_ASSERT(format != NULL);

    usize count = 0;

    while (*format)
    {
        if (*format++ == '%')
        {
            if (*format == '%')
            {
                ++format;
                continue;
            }

            ++count;
        }
    }

    return count;
}

Diagnostic* diagnostic_create(DiagnosticID id, DiagnosticLevel lvl,
                              SourceSpan span, ...)
{
    FRX_ASSERT(id < FRX_DIAGNOSTIC_ID_COUNT);
    FRX_ASSERT(lvl < FRX_DIAGNOSTIC_LVL_COUNT);

    Diagnostic* d = compiler_alloc(sizeof(Diagnostic));

    d->id = id;
    d->lvl = lvl;
    d->span = span;

    usize args_count = format_args_count(diagnostic_id_to_format[d->id]);

    FRX_ASSERT(args_count <= FRX_DIAGNOSTICS_MAX_ARGS);

    if (args_count > 0)
    {
        va_list args;
        va_start(args, span);

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
    FRX_ASSERT(d != NULL);

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
    const char* filepath = source_map_filepath_from_source_span(d->span);

    SourceLine line;
    SourceColumn column;
    const char* src = source_map_resolve_offset(d->span.lo, &line, &column);

    fprintf(output, "[%s%s%s]: %s:%u:%u: ", color_str, lvl_str,
            clear_color_str, filepath, line, column);

    const char* format = diagnostic_id_to_format[d->id];
    usize args_count = format_args_count(format);

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

    const char* line_start = src - column + 1;
    usize line_len = 1;
    while (*line_start != '\n' && *line_start != '\0')
    {
        ++line_start;
        ++line_len;
    }

    line_start = src - column + 1;

    static const int line_format_width = 5;

    fprintf(output, "%*d | %.*s\n", line_format_width, line, (int)(line_len - 1), line_start);
    fprintf(output, "%*s | %*s^", line_format_width, "", (int)(column - 1), "");

    for (usize i = 1; i < d->span.hi - d->span.lo; ++i)
    {
        printf("~");
    }

    printf("\n");
}
