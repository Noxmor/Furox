#define CBREW_ENABLE_ASSERTS
#define CBREW_ENABLE_CONSOLE_COLORS
#define CBREW_IMPLEMENTATION

#include "cbrew.h"

#define SYSTEM_NAME CBREW_ARCHITECTURE_NAME "-" CBREW_PLATFORM_NAME

void create_furox_debug_config(CbrewProject* project)
{   
    CbrewConfig* debug = CBREW_CFG_NEW(project, "Debug", "bin/Furox-Debug-" SYSTEM_NAME, "bin-int/Furox-Debug-" SYSTEM_NAME);
    CBREW_CFG_DEFINE(debug, "FRX_DEBUG");
    CBREW_CFG_DEFINE(debug, "FRX_ENABLE_ASSERTS");
    CBREW_CFG_FLAG(debug, "-ggdb");
    CBREW_CFG_FLAG(debug, "-O0");
}

void create_furox_release_config(CbrewProject* project)
{
    CbrewConfig* release = CBREW_CFG_NEW(project, "Release", "bin/Furox-Release-" SYSTEM_NAME, "bin-int/Furox-Release-" SYSTEM_NAME);
    CBREW_CFG_DEFINE(release, "FRX_RELEASE");
    CBREW_CFG_DEFINE(release, "FRX_ENABLE_ASSERTS");
    CBREW_CFG_FLAG(release, "-ggdb");
    CBREW_CFG_FLAG(release, "-O3");
}

void create_furox_dist_config(CbrewProject* project)
{
    CbrewConfig* dist = CBREW_CFG_NEW(project, "Dist", "bin/Furox-Dist-" SYSTEM_NAME, "bin-int/Furox-Dist-" SYSTEM_NAME);
    CBREW_CFG_DEFINE(dist, "FRX_DIST");
}

void create_furox_project(void)
{
    CbrewProject* project = CBREW_PRJ_NEW("Furox", CBREW_PROJECT_TYPE_APP);
    CBREW_PRJ_FILES(project, "./src/**.c");
    CBREW_PRJ_INCLUDE_DIR(project, "src");
    CBREW_PRJ_FLAG(project, "-Wall");
    CBREW_PRJ_FLAG(project, "-Wextra");
    CBREW_PRJ_FLAG(project, "-Wunreachable-code");

    create_furox_debug_config(project);
    create_furox_release_config(project);
    create_furox_dist_config(project);
}

int main(int argc, char** argv)
{
    CBREW_AUTO_REBUILD(argc, argv);

    create_furox_project();

    cbrew_build(argc, argv);
    
    return EXIT_SUCCESS;
}
