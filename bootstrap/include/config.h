#ifndef FRX_CONFIG_H
#define FRX_CONFIG_H

#include "types.h"

typedef struct Config
{
    const char* output;
    b8 use_stdlib;
} Config;

void config_parse(int argc, char** argv);

const Config* config_get(void);

int config_get_optind(void);

#endif
