#ifndef FRX_MODULE_H
#define FRX_MODULE_H

#include <dirent.h>

#include "list.h"

typedef struct Module
{
    struct Module* parent;
    char filepath[PATH_MAX];
    const char* name;
    b8 failed;
    List submodules;
    List parsers;
} Module;

Module* module_create(const char* project_path);

void module_compile(Module* mod);

void module_codegen(Module* mod);

b8 module_failed(const Module* mod);

void module_destroy(Module* mod);

#endif
