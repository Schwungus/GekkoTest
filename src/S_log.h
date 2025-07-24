#pragma once

#include <stdio.h>

#define _LOG(level, file, line, fmt, ...)                                                                              \
    do {                                                                                                               \
        printf("%s [%s:%d]: " fmt "\n", level, file, line, ##__VA_ARGS__);                                             \
        fflush(stdout);                                                                                                \
    } while (0)

#define LOG(level, ...) _LOG(level, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(...) LOG("INFO", __VA_ARGS__)
