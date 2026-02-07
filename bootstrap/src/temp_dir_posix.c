#include "temp_dir.h"

#include "platform_detection.h"

#ifdef FRX_PLATFORM_POSIX

#include "assert.h"
#include "types.h"

#include <stdlib.h>

static b8 initialized;
static const char* temp_dir;

void temp_dir_init(void)
{
    initialized = FRX_TRUE;

    temp_dir = getenv("TMPDIR");
    if (temp_dir == NULL)
    {
        temp_dir = "/tmp";
    }
}

const char* temp_dir_path(void)
{
    FRX_ASSERT(initialized);

    return temp_dir;
}

#endif
