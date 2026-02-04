#include "resolution.h"

#include "assert.h"
#include "log.h"

void resolution_context_init(ResolutionContext* ctx, SourceFile* src_file,
                             Module* root_mod)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(root_mod != NULL);

    if (src_file != NULL)
    {
        FRX_LOG_INFO("Initializing resolution context for file: %s...", src_file->path);
    }

    ctx->src_file = src_file;
    ctx->root_mod = root_mod;
    ctx->current_scope = src_file != NULL ? src_file->global_scope : NULL;
    ctx->failed = FRX_FALSE;
}

void resolution_context_push_scope(ResolutionContext* ctx, Scope* scope)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(scope != NULL);

    ctx->current_scope = scope;
}

void resolution_context_pop_scope(ResolutionContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(ctx->current_scope != NULL);

    ctx->current_scope = ctx->current_scope->parent;
}

Symbol* resolution_context_lookup_symbol(ResolutionContext* ctx, const char* name)
{
    FRX_ASSERT(ctx != NULL);

    return scope_lookup_symbol(ctx->current_scope, name);
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
