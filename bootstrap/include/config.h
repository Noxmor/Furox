#ifndef FRX_CONFIG_H
#define FRX_CONFIG_H

typedef struct Config
{
    const char* output;
} Config;

void config_parse(int argc, char** argv);

const Config* config_get(void);

int config_get_optind(void);

#endif
