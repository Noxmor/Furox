#include "sema.h"

#include "assert.h"
#include "log.h"

void sema_context_init(SemaContext* ctx, const SourceFile* src_file)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(src_file != NULL);

    FRX_LOG_INFO("Initializing sema context for file: %s...", src_file->path);

    ctx->src_file = src_file;
    ctx->current_impl_block = NULL;
    ctx->failed = FRX_FALSE;
}

void sema_context_fail(SemaContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    ctx->failed = FRX_TRUE;
}

b8 sema_context_failed(const SemaContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    return ctx->failed;
}
