#ifndef FRX_PROJECT_H
#define FRX_PROJECT_H

#include "project_specification.h"
#include "module.h"

typedef struct Project
{
    ProjectSpecificiation specification;
    Module* root_module;
} Project;

Project* project_create(ProjectSpecificiation spec, const char* project_path);

void project_compile(Project* project);

Module* project_find_module_by_path_segments(Project* project, const List* path_segments);

b8 project_failed(const Project* project);

void project_destroy(Project* project);

#endif
