#ifndef FRX_PLATFORM_DETECTION_H
#define FRX_PLATFORM_DETECTION_H

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
#define FRX_PLATFORM_WINDOWS
#ifdef _WIN64
#define FRX_PLATFORM_WIN64
#else
#define FRX_PLATFORM_WIN32
#endif
#elif defined(__linux__)
#define FRX_PLATFORM_LINUX
#define FRX_PLATFORM_POSIX
#elif defined(__APPLE__) && defined(__MACH__)
#define FRX_PLATFORM_MACOS
#define FRX_PLATFORM_POSIX
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#define FRX_PLATFORM_MACOS_X
#endif
#elif defined(__unix__)
#define FRX_PLATFORM_UNIX
#define FRX_PLATFORM_POSIX
#elif defined(__POSIX__)
#define FRX_PLATFORM_POSIX
#elif defined(__ANDROID__)
#define FRX_PLATFORM_ANDROID
#elif defined(__FreeBSD__)
#define FRX_PLATFORM_FREEBSD
#define FRX_PLATFORM_POSIX
#else
#define FRX_PLATFORM_UNKNOWN
#endif

#endif
