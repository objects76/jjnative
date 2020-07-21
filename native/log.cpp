
#include "log.h"
#include <stdarg.h>
#include <string>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

bool isDev = true;

// std::string format(const char *fmt, ...)
// {
//     char buffer[4096];
//     va_list args;
//     va_start(args, fmt);
//     int n = vsprintf_s(buffer, sizeof(buffer) - 1, fmt, args);
//     va_end(args);

//     return std::string(buffer, n);
// }

// void log(const char *fmt, ...)
// {
//     char buffer[4096];
//     va_list args;
//     va_start(args, fmt);
//     int n = vsprintf_s(buffer, sizeof(buffer) - 1, fmt, args);
//     va_end(args);
// #ifdef WIN32
//     ::OutputDebugStringA(buffer);
//     ::fputs(buffer, stdout);
// #else
//     ::fputs(buffer, stdout);
// #endif
// }
