#include <stdlib.h>

#include "log.h"
#include "lexer.h"
#include "string_table.h"

static void compiler_init(void)
{
    FRX_LOG_INFO("Initializing compiler...");

    lexer_init_keyword_table();
}

static void compiler_shutdown(void)
{
    FRX_LOG_INFO("Shutting down compiler...");

    string_table_shutdown();
}

int compiler_run(int argc, char** argv)
{
    compiler_init();

    compiler_shutdown();

    return EXIT_SUCCESS;
}
