#include <stdlib.h>

#include "log.h"

static void compiler_init(void)
{
    FRX_LOG_INFO("Initializing compiler...");
}

static void compiler_shutdown(void)
{
    FRX_LOG_INFO("Shutting down compiler...");
}

int compiler_run(int argc, char** argv)
{
    compiler_init();

    compiler_shutdown();

    return EXIT_SUCCESS;
}
