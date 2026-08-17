#ifndef LAFVIN_EPD_DEBUG_H
#define LAFVIN_EPD_DEBUG_H

#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define Debug(...)                    \
    do {                              \
        printf("Debug: ");           \
        printf(__VA_ARGS__);          \
    } while (0)
#else
#define Debug(...)                    \
    do {                              \
        if (0) {                      \
            printf(__VA_ARGS__);      \
        }                             \
    } while (0)
#endif

#endif
