#pragma once
#include <string>

void throw_error(const std::string &msg, const std::string &pos);
void fatal_error(const std::string &msg, const std::string &pos);
#define Assert1(c, msg)                     \
    do                                      \
    {                                       \
        if (!(c))                           \
            fatal_error(msg, __FUNCTION__); \
    } while (0)

#define Assert0(c)                                              \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
            fatal_error("assertion failed: " #c, __FUNCTION__); \
    } while (0)
