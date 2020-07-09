
#pragma once
#include <stdarg.h>
#include <string>

inline std::string format(const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    int n = vsprintf_s(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    return std::string(buffer, n);
}

inline void log(const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    int n = vsprintf_s(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);
#ifdef WIN32
    ::OutputDebugStringA(buffer);
#else
    ::fputs(buffer, stdout);
#endif
}

#define TOSTR2(s) #s
#define TOSTR(s) TOSTR2(s)
