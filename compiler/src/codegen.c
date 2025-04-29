#include "codegen.h"

#include <string.h>

u8 codegen_context_begin(CodegenContext* ctx, const char* name)
{
    char buffer[strlen("/tmp/") + strlen(name) + strlen(".c") + 1];

    sprintf(buffer, "/tmp/%s.c", name);
    ctx->source = fopen(buffer, "w");

    if (ctx->source == NULL)
    {
        return FRX_TRUE;
    }

    sprintf(buffer, "/tmp/%s.h", name);
    ctx->header = fopen(buffer, "w");

    if (ctx->header == NULL)
    {
        fclose(ctx->source);
        return FRX_TRUE;
    }

    fprintf(ctx->header, "#ifndef FRX_%s_H\n", name);
    fprintf(ctx->header, "#define FRX_%s_H\n", name);

    fprintf(ctx->header, "#include <stddef.h>\n");
    fprintf(ctx->header, "#include <stdint.h>\n");

    fprintf(ctx->header, "typedef int8_t i8;\n");
    fprintf(ctx->header, "typedef int16_t i16;\n");
    fprintf(ctx->header, "typedef int32_t i32;\n");
    fprintf(ctx->header, "typedef int64_t i64;\n");
    fprintf(ctx->header, "typedef int64_t isize;\n");

    fprintf(ctx->header, "typedef uint8_t u8;\n");
    fprintf(ctx->header, "typedef uint16_t u16;\n");
    fprintf(ctx->header, "typedef uint32_t u32;\n");
    fprintf(ctx->header, "typedef uint64_t u64;\n");
    fprintf(ctx->header, "typedef uint64_t usize;\n");

    fprintf(ctx->header, "typedef u8 b8;\n");
    fprintf(ctx->header, "typedef u16 b16;\n");
    fprintf(ctx->header, "typedef u32 b32;\n");
    fprintf(ctx->header, "typedef u64 b64;\n");

    fprintf(ctx->header, "typedef float f32;\n");
    fprintf(ctx->header, "typedef double f64;\n");

    fprintf(ctx->source, "#include \"%s.h\"\n", name);

    return FRX_FALSE;
}

void codegen_context_end(CodegenContext* ctx)
{
    fprintf(ctx->header, "#endif");

    fflush(ctx->header);
    fflush(ctx->source);

    fclose(ctx->source);
    fclose(ctx->header);
}
