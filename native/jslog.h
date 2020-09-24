
#pragma once
#include <stdarg.h>
#include <string>
#include <filesystem>
#include <sstream>
#include <string_view>

//
// native related
//
extern bool isDev;
#include "utils/logger.h"

#define devlog(...) \
    if (isDev)      \
    LOGV << fmt::csprintf(__VA_ARGS__)

void js_throw_error(std::string_view pos, std::string_view msg = "");
void js_fatal_error(std::string_view pos, std::string_view msg = "");

#define JSLOG(fmt, ...) jslog(__basename__, __LINE__, fmt, ##__VA_ARGS__)
void jslog(const char *pos, int line, const char *fmt, ...);