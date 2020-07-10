
#pragma once
#include <stdarg.h>
#include <string>

//
// native related
//
extern bool isDev;
std::string format(const char *fmt, ...);
void log(const char *fmt, ...);

#define devlog(...)           \
    do                        \
    {                         \
        if (isDev)            \
            log(__VA_ARGS__); \
    } while (0)

//
// napi related.
//
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

//
// macros
//
#define TOSTR2(s) #s
#define TOSTR(s) TOSTR2(s)
