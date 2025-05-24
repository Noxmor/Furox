#ifndef FRX_SOURCE_REGISTRY_H
#define FRX_SOURCE_REGISTRY_H

#include "source_file.h"

SourceFileID source_registry_load(const char* filepath);

const SourceFile* source_registry_get(SourceFileID id);

#endif
