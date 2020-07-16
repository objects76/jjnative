
#pragma once
#include <stdarg.h>
#include <string>
#include <filesystem>

//
// native related
//
extern bool isDev;
std::string format(const char *fmt, ...);
void log(const char *fmt, ...);
inline void log() {}

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
#define TOSTR2(s)       #s
#define TOSTR(s)        TOSTR2(s)
#define CONCAT_(X,Y)	X##Y
#define CONCAT(X,Y)		CONCAT_(X,Y)

#define LOG(...) do{ \
        log(__VA_ARGS__);                        \
        log(" at %s:%d\n", __basename__, __LINE__); \
    } while(0)

#define __basename__	([]{ constexpr auto x = klog::filename(__FILE__); return x; }())
namespace klog {
    namespace {
        using namespace std::filesystem;
        constexpr const char* eos(const char *str) { return (*str != 0) ? eos(str + 1) : str; }
        constexpr bool is_path(const char *str) {
            return (*str == path::preferred_separator) ? true : (*str ? is_path(str + 1) : false);
        }
        constexpr const char* basename(const char* str) {
            return (*str == path::preferred_separator) ? (str + 1) : basename(str - 1);
        }
    }
	constexpr const char* filename(const char* str) {
		return is_path(str) ? basename(eos(str)) : str;
	}

    struct FnScope190604 {
        const char* _label;
        FnScope190604(const char* s) : _label(s) { log("{ %s\n", _label); }
        ~FnScope190604() { log("} %s\n", _label); }
    };

}

#define FNSCOPE()		    klog::FnScope190604 CONCAT(fn190604,__LINE__)(__FUNCTION__)
#ifdef WIN32
#define LogOnFalse(op)      do{ if (!(op)) LOG("'%s' failed: errno=%d", #op, ::GetLastError()); } while(0)
#else
#define LogOnFalse(op)      do{ if (!(op)) LOG("'%s' failed: errno=%d", #op, errno); } while(0)
#endif