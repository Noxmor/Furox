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

b8 project_failed(const Project* project);

void project_destroy(Project* project);

#endif
