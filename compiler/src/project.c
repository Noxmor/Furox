#include "project.h"

#include "assert.h"
#include "compiler.h"

Project* project_create(ProjectSpecificiation spec, const char* project_path)
{
    Project* project = compiler_alloc(sizeof(Project));

    project->specification = spec;
    project->root_module = module_create(project_path);

    return project;
}

void project_compile(Project* project)
{
    FRX_ASSERT(project != NULL);

    module_compile(project->root_module);
}

b8 project_failed(const Project* project)
{
    FRX_ASSERT(project != NULL);

    return module_failed(project->root_module);
}

void project_destroy(Project* project)
{
    FRX_ASSERT(project != NULL);

    module_destroy(project->root_module);
}
