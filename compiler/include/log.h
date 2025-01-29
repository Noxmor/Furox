#ifndef FRX_LOG_H
#define FRX_LOG_H

#ifdef FRX_DEBUG
#include <stdio.h>
#define FRX_LOG_ERROR(format, ...) do { fprintf(stderr, "[ERROR]: "); fprintf(stderr, format, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define FRX_LOG_WARN(format, ...) do { fprintf(stderr, "[WARN]: "); fprintf(stderr, format, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define FRX_LOG_INFO(format, ...) do { printf("[INFO]: "); printf(format, ##__VA_ARGS__); printf("\n"); } while(0)
#define FRX_LOG_DEBUG(format, ...) do { printf("[DEBUG]: "); printf(format, ##__VA_ARGS__); printf("\n"); } while(0)
#else
#define FRX_LOG_ERROR(format, ...)
#define FRX_LOG_WARN(format, ...)
#define FRX_LOG_INFO(format, ...)
#define FRX_LOG_DEBUG(format, ...)
#endif

#endif
