#pragma once

#include <stdio.h>

#define INFO(fmt, ...)                                                                                                 \
    printf(fmt "\n", ##__VA_ARGS__);                                                                                   \
    fflush(stdout);
