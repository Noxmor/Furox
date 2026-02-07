#include "temp_dir.h"

#include "platform_detection.h"

#ifdef FRX_PLATFORM_WINDOWS

static b8 initialized;
static char temp_dir[MAX_PATH];

void temp_dir_init(void)
{
    initialized = FRX_TRUE;

    GetTempPathA(sizeof(temp_dir), temp_dir);
}

const char* temp_dir_path(void)
{
    FRX_ASSERT(initialized);

    return temp_dir;
}

#endif
