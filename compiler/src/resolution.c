#include "resolution.h"

#include "assert.h"
#include "log.h"

void resolution_context_init(ResolutionContext* ctx, SourceFile* src_file)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(src_file != NULL);

    FRX_LOG_INFO("Initializing resolution context for file: %s...", src_file->path);

    ctx->src_file = src_file;
    ctx->failed = FRX_FALSE;
}

Symbol* resolution_context_lookup_symbol(ResolutionContext* ctx, SymbolType type,
                                         const char* name)
{
    FRX_ASSERT(ctx != NULL);

    return source_file_lookup_symbol(ctx->src_file, type, name);
}

void resolution_context_fail(ResolutionContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    ctx->failed = FRX_TRUE;
}

b8 resolution_context_failed(const ResolutionContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    return ctx->failed;
}
