#include "project.h"

#include <string.h>

#include "assert.h"
#include "codegen.h"
#include "compiler.h"
#include "module.h"
#include "project_specification.h"

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

u8 project_codegen(Project* project)
{
    FRX_ASSERT(project != NULL);

    CodegenContext ctx;
    codegen_context_begin(&ctx, project->root_module->name);

    module_codegen(project->root_module, &ctx);

    codegen_context_end(&ctx);

    if (project->specification.type == FRX_PROJECT_TYPE_APP)
    {
        char buffer[strlen("gcc /tmp/") + strlen(project->root_module->name) + strlen(".c") + 1];
        sprintf(buffer, "gcc /tmp/%s.c", project->root_module->name);

        if (system(buffer) != 0)
        {
            return FRX_TRUE;
        }
    }

    return FRX_FALSE;
}

Module* project_find_module_by_path_segments(Project* project, const List* path_segments)
{
    FRX_ASSERT(project != NULL);

    FRX_ASSERT(path_segments != NULL);

    FRX_ASSERT(!list_empty(path_segments));

    Module* mod = project->root_module;
    if (strcmp(mod->name, list_get(path_segments, 0)) != 0)
    {
        return NULL;
    }

    for (usize i = 1; mod != NULL && i < list_size(path_segments); ++i)
    {
        mod = module_find_submodule_by_name(mod, list_get(path_segments, i));
    }

    return mod;
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
