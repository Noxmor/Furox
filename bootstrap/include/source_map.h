#ifndef FRX_SOURCE_MAP_H
#define FRX_SOURCE_MAP_H

#include "source_span.h"
#include "list.h"

void source_map_init(void);

void source_map_add_source_file(const char* filepath);

void source_map_resolve_offset(SourceOffset offset, SourceLine* line, SourceColumn* column);

const char* source_map_filepath_from_source_span(SourceSpan span);

List* source_map_get_source_files(void);

#endif
