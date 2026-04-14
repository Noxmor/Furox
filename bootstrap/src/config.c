#include "assert.h"
#include "config.h"
#include "types.h"
#include "version.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#define FRX_HELP_MESSAGE_WIDTH 80

#define FRX_OPTIONS(X) \
    X("help", 'h', no_argument, "Show this help message.") \
    X("version", 'v', no_argument, "Show compiler version.") \
    X("output", 'o', required_argument, "Set output name to <file>.")

#define FRX_GENERATE_LONG_OPTIONS(long_name, short_name, arg, desc) \
    { long_name, arg, 0, short_name },

#define FRX_GENERATE_HELP_STR(long_name, short_name, arg, desc) \
    print_option(long_name, short_name, desc);

#define FRX_GENERATE_COLUMN_WIDTH(long_name, short_name, arg, desc) \
    do \
    { \
        usize width = 0; \
        compute_desc_arg_and_len(desc, &width); \
        width += sizeof("  -#, --") - 1 + sizeof(long_name) - 1 + sizeof(" ") - 1; \
        if (width > column_width) column_width = width; \
    } while (FRX_FALSE);

static usize column_width = 0;

static Config config = {
    .output = "a.out"
};

static const char* compute_desc_arg_and_len(const char* desc, usize* len)
{
    FRX_ASSERT(desc != NULL);

    FRX_ASSERT(len != NULL);

    usize arg_len = 0;

    while (*desc++ != '\0')
    {
        if (*desc == '<' || *desc == '[')
        {
            while (desc[arg_len] != '>' && desc[arg_len] != ']')
            {
                ++arg_len;
            }

            *len = ++arg_len;
            return desc;
        }
    }

    return NULL;
}

static void print_option(const char* long_name, char short_name, const char* desc)
{
    usize column = 0;

    if (long_name != NULL && short_name != '\0')
    {
        column += printf("  -%c, --%s", short_name, long_name);
    }
    else if (long_name != NULL)
    {
        column += printf("      --%s", long_name);
    }
    else if (short_name != '\0')
    {
        column += printf("  -%c", short_name);
    }
    else
    {
        FRX_ASSERT(FRX_FALSE);
    }

    usize arg_len = 0;
    const char* arg = compute_desc_arg_and_len(desc, &arg_len);

    if (arg != NULL)
    {
        column += printf(" %.*s", (int)arg_len, arg);
    }

    printf("%*s", (int)(column_width - column), "");

    column = column_width;

    while (*desc != '\0')
    {
        while (*desc == ' ')
        {
            ++desc;
        }

        const char* word = desc;

        while (*desc != ' ' && *desc != '\0')
        {
            ++desc;
        }

        usize len = desc - word;

        if (column + len >= FRX_HELP_MESSAGE_WIDTH)
        {
            column = column_width;
            printf("\n");
            printf("%*s", (int)column_width, "");
        }

        column += printf(" %.*s", (int)len, word);
    }

    printf("\n");
}

static void print_help(const char* program)
{
    FRX_ASSERT(program != NULL);

    FRX_OPTIONS(FRX_GENERATE_COLUMN_WIDTH);

    printf("Usage: %s [OPTION]... FILE...\n\nOptions:\n", program);
    FRX_OPTIONS(FRX_GENERATE_HELP_STR);
}

void config_parse(int argc, char** argv)
{
    FRX_ASSERT(argv != NULL);

    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        FRX_OPTIONS(FRX_GENERATE_LONG_OPTIONS)
        { 0, 0, 0, 0 }
    };

    while ((opt = getopt_long(argc, argv, "hvo:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 'h': print_help(argv[0]); exit(0);
            case 'v': printf("version %s\n", FRX_VERSION_STR); exit(0);
            case 'o': config.output = optarg; break;

            case '?': exit(EXIT_FAILURE);

            default: FRX_ASSERT(FRX_FALSE); break;
        }
    }
}

const Config* config_get(void)
{
    return &config;
}

int config_get_optind(void)
{
    return optind;
}
