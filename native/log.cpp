
#include "log.h"
#include <stdarg.h>
#include <string>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

bool isDev = true;
