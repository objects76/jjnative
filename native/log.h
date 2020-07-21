
#pragma once
#include <stdarg.h>
#include <string>
#include <filesystem>
#include <sstream>
#include <string_view>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <errno.h>
#define GetLastError()  (errno)
#endif
//
// native related
//
extern bool isDev;
#include "logger.h"

#define devlog(...) if (isDev) LOGV << fmt::csprintf(__VA_ARGS__)
// std::string format(const char *fmt, ...);
// void log(const char *fmt, ...);
// inline void log() {}

// #define devlog(...)           \
//     do                        \
//     {                         \
//         if (isDev)            \
//             log(__VA_ARGS__); \
//     } while (0)

// //
// // napi related.
// //

// namespace va {
// 	template<typename ...Args>
// 	constexpr std::size_t count(Args&&...) { return sizeof...(Args); }
// }

void throw_error(std::string_view pos, std::string_view msg = "");
void fatal_error(std::string_view pos, std::string_view msg = "");
// #define Assert(c, ...)	\
// 	if (!(c)) { \
// 		if constexpr (va::count(##__VA_ARGS__) == 0) fatal_error(__FUNCTION__, "assertion: " #c); \
// 		else fatal_error(__FUNCTION__, ##__VA_ARGS__); \
// 	}

// //
// // macros
// //
// #define TOSTR2(s)       #s
// #define TOSTR(s)        TOSTR2(s)
// #define CONCAT_(X,Y)	X##Y
// #define CONCAT(X,Y)		CONCAT_(X,Y)

// #define LOG(...)    do { log("%s at %s:%d\n", format(__VA_ARGS__).c_str(), __basename__, __LINE__); } while(0)


// //
// // FNSCOPE();
// //
// #define FNSCOPE()		    klog::FnScope CONCAT(fn190604,__LINE__)(__FUNCTION__)

// #define __basename__	([]{ constexpr auto x = klog::filename(__FILE__); return x; }())
// namespace klog {
//     namespace {
//         using namespace std::filesystem;
//         constexpr const char* eos(const char *str) { return (*str != 0) ? eos(str + 1) : str; }
//         constexpr bool is_path(const char *str) {
//             return (*str == path::preferred_separator) ? true : (*str ? is_path(str + 1) : false);
//         }
//         constexpr const char* basename(const char* str) {
//             return (*str == path::preferred_separator) ? (str + 1) : basename(str - 1);
//         }
//     }
// 	constexpr const char* filename(const char* str) {
// 		return is_path(str) ? basename(eos(str)) : str;
// 	}

//     struct FnScope {
//         const char* _label;
//         FnScope(const char* s) : _label(s) { log("{ %s\n", _label); }
//         ~FnScope() { log("} %s\n", _label); }
//     };

// }



// //
// // EXPECT(op, expected) or EXPECT(op_boolable)
// // EXPECT_NOT(op, not_expected)
// //
// #define EXPECT(op, ...)                   klog::expect_(#op, __LINE__, (op), ##__VA_ARGS__)
// #define EXPECT_NOT(op, not_expected)      klog::expect_not_(#op, __LINE__, (op), not_expected)
// namespace klog {
//     template<typename T>
//     inline T expect_(const char* msg, int line, const T value, const T expected)
//     {
//         if (value != expected) {
//             std::ostringstream str;
//             str << "expect "<<expected <<", but " << value <<", errno="<< GetLastError() << " at " << msg <<":"<<line << "\n"; 
//             log(str.str().c_str());
//         }
//         return value;
//     }

//     template<typename T>
//     inline T expect_(const char* msg, int line, T value )
//     {
//         if (!value) {
//             std::ostringstream str;
//             str << "Failed, errno="<< GetLastError() << " at " << msg <<":"<<line << "\n";
//             log(str.str().c_str());        
//         }
//         return value;
//     }


//     template<typename T>
//     inline T expect_not_(const char* msg, int line, T value, const T expected)
//     {
//         if (value == expected) {
//             std::ostringstream str;
//             str << "Not expect "<< expected <<", errno="<<GetLastError() << " at " << msg <<":"<<line << "\n"; 
//             log(str.str().c_str());
//         }
//         return value;
//     }

// } // klog
