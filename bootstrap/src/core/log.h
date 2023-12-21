#ifndef FRX_LOG_H
#define FRX_LOG_H

#include <stdio.h>
#include <stdlib.h>

#ifdef FRX_DEBUG
#define FRX_TRACE(format, ...) printf("[TRACE]: " format "\n", ##__VA_ARGS__)
#define FRX_TRACE_FILE(format, file, line, coloumn, ...) printf("[TRACE]: %s:%zu:%zu: " format "\n", file, line, coloumn, ##__VA_ARGS__)
#else
#define FRX_TRACE(format, ...)
#define FRX_TRACE_FILE(format, file, line, coloumn, ...)
#endif

#define FRX_INFO(format, ...) printf("[INFO]: " format "\n", ##__VA_ARGS__)
#define FRX_INFO_FILE(format, file, line, coloumn, ...) printf("[INFO]: %s:%zu:%zu: " format "\n", file, line, coloumn, ##__VA_ARGS__)

#define FRX_WARN(format, ...) fprintf(stderr, "[WARN]: " format "\n", ##__VA_ARGS__)
#define FRX_WARN_FILE(format, file, line, coloumn, ...) fprintf(stderr, "[WARN]: %s:%zu:%zu: " format "\n", file, line, coloumn, ##__VA_ARGS__)

#define FRX_ERROR(format, ...) fprintf(stderr, "[ERROR]: " format "\n", ##__VA_ARGS__)
#define FRX_ERROR_FILE(format, file, line, coloumn, ...) fprintf(stderr, "[ERROR]: %s:%zu:%zu: " format "\n", file, line, coloumn, ##__VA_ARGS__)

#define FRX_FATAL(format, ...) do { fprintf(stderr, "[FATAL]: " format "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)
#define FRX_FATAL_FILE(format, file, line, coloumn, ...) do { fprintf(stderr, "[FATAL]: %s:%zu:%zu: " format "\n", file, line, coloumn, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

#endif
