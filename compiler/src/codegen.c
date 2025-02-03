#include "codegen.h"

#include <stdarg.h>
#include <stdio.h>

#include "assert.h"

static FILE* file;

b8 codegen_begin(const char* output)
{
    FRX_ASSERT(output != NULL);

    file = fopen(output, "w");
    if(file == NULL)
    {
        return FRX_TRUE;
    }

    codegen_write("#include <stddef.h>\n");
    codegen_write("#include <stdint.h>\n");

    return FRX_FALSE;
}

void codegen_write(const char* format, ...)
{
    FRX_ASSERT(format != NULL);

    va_list args;
    va_start(args, format);

    vfprintf(file, format, args);

    va_end(args);
}

void codegen_end(void)
{
    fclose(file);
}
