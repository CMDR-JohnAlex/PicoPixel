#pragma once

// TODO: Add to CMake.
#ifndef STRIP_LOGGING
    #include <cstdio>
    #include <cstring>
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #define LOG(fmt, ...) printf("[LOG] %s:%d %s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...)
#endif
