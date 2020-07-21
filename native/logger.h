#pragma once
#include <memory>
#include <iostream>
#include <chrono>
#include <array>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <sstream>


#define STANDALONE

#ifdef WIN32
#pragma comment(lib, "Wtsapi32")
#endif

#define LOGV			klog::LogStream('V', __basename__,__LINE__).stream()
#define LOGD			klog::LogStream('D', __basename__,__LINE__).stream()
#define LOGI			klog::LogStream('I', __basename__,__LINE__).stream()
#define LOGW			klog::LogStream('W', __basename__,__LINE__).stream()
#define LOGE			klog::LogStream('E', __basename__,__LINE__).stream()
#define LOGV0			klog::LogStream('V', 0,0).stream()
#define LOGD0			klog::LogStream('D', 0,0).stream()
#define LOGI0			klog::LogStream('I', 0,0).stream()
#define LOGW0			klog::LogStream('W', 0,0).stream()
#define LOGE0			klog::LogStream('E', 0,0).stream()

#define LOGV_IF(c)	if (c) LOGV
#define LOGD_IF(c)	if (c) LOGD
#define LOGI_IF(c)	if (c) LOGI
#define LOGW_IF(c)	if (c) LOGW
#define LOGE_IF(c)	if (c) LOGE

#define __func2__	__FUNCTION__
#define FNSCOPE_IF(c)	std::unique_ptr<klog::FnScope> CONCAT(pfn190604,__LINE__) = c ? std::make_unique<klog::FnScope>(__func2__):nullptr
#define FNSCOPE()		klog::FnScope CONCAT(fn190604,__LINE__)(__func2__)
#define MSGSCOPE(str)	klog::FnScope CONCAT(ms190604,__LINE__)(str)

#define LOGFMT(fmt, ...)		klog::DebugPrintf(__basename__, __LINE__, fmt, ##__VA_ARGS__)


// {...} : errno.
#define Assert(c)		if (!(c)) klog::AssertStream(__basename__, __LINE__, GetLastError()).stream() << "Assert: " #c << '\n'

// {...} : errno.
#define LOGERRNO(...)	klog::ErrnoStream(__basename__, __LINE__, ##__VA_ARGS__).stream()
//#define LOGERRNO(...)	CONCAT(logerrno_, GET_ARG_COUNT(__VA_ARGS__)) (__VA_ARGS__)
//#define		logerrno_0()	klog::ErrnoStream(__basename__, __LINE__, GetLastError()).stream()
//#define		logerrno_1(en)	klog::ErrnoStream(__basename__, __LINE__, en).stream()

// {...} : errno.
#define exmsg(msg)				(std::string(msg) + " at " __func2__ "." TOSTR(__LINE__))
// #define syserror(msg, ...)		CONCAT(syserror_, GET_ARG_COUNT(__VA_ARGS__)) (msg, ##__VA_ARGS__)
// #define		syserror_0(msg,...)		std::system_error(GetLastError(), error_category(), exmsg(msg))
// #define		syserror_1(msg, en)		std::system_error(en, error_category(), exmsg(msg))

#ifdef _WIN32
#define error_category()		klog::win32_category()
#else
#define error_category()		std::system_category()
#endif


#ifndef CONCAT
# define CONCAT2(x,y)				x##y
# define CONCAT(x,y)					CONCAT2(x,y)
# define TOSTR2(x)					#x
# define TOSTR(x)					TOSTR2(x)
# define EXPAND(x) x
# ifdef _MSC_VER
#   define GET_ARG_COUNT(...)  GET_ARG_COUNT_(ARGS_AUGMENTER_(__VA_ARGS__))
#   define ARGS_AUGMENTER_(...) unused, __VA_ARGS__
#   define GET_ARG_COUNT_(...) EXPAND(GET_ARG_COUNT_2(__VA_ARGS__,  9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define GET_ARG_COUNT_2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, count, ...) count
# else
#   define GET_ARG_COUNT(...) GET_ARG_COUNT_2(0, ## __VA_ARGS__,  10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#   define GET_ARG_COUNT_2(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, count, ...) count
# endif
#endif

#ifdef _WIN32
# undef  _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
# include <comdef.h>
# include <process.h>
  inline int gettid() { return GetCurrentThreadId(); }
  inline int getPid() { return GetCurrentProcessId(); }
#else

inline void OutputDebugStringA(const char* msg) { printf(msg); }
inline void OutputDebugStringW(const wchar_t* msg) { wprintf(msg); }
#endif


#ifndef __basename__
#define __basename__	([]{ constexpr auto x = klog::filename(__FILE__); return x; }())
namespace klog {
#ifdef _WIN32
	constexpr const char pathsep = '\\';
#else
	constexpr const char pathsep = '/';
#endif
	constexpr const char* eos(const char *str) { return (*str != 0) ? eos(str + 1) : str; }
	constexpr bool is_path(const char *str) {
		return (*str == pathsep) ? true : (*str ? is_path(str + 1) : false);
	}
	constexpr const char* basename(const char* str) {
		return (*str == pathsep) ? (str + 1) : basename(str - 1);
	}
	constexpr const char* filename(const char* str) {
		return is_path(str) ? basename(eos(str)) : str;
	}
}
#endif



#include <stdarg.h>
namespace fmt
{
	inline std::string csprintf(const char* fmt, ...)
	{
		char buffer[4096];
		va_list args;
		va_start(args, fmt);
		int n = vsprintf_s(buffer, sizeof(buffer) - 1, fmt, args);
		va_end(args);

		return std::string(buffer, n);
	}


	inline std::wstring csprintf(const wchar_t* fmt, ...)
	{
		wchar_t buffer[4096];
		va_list args;
		va_start(args, fmt);
		int n = vswprintf_s(buffer, sizeof(buffer) / 2, fmt, args);
		va_end(args);

		return std::wstring(buffer, n);
	}
}


#ifndef TAG
#define kTag	"    "
#else
#define kTag	TOSTR(TAG)
#endif


//
//
//
namespace klog
{
	struct Out
	{
		inline static void(__stdcall* A)(const char*) = &OutputDebugStringA;
		inline static void(__stdcall* W)(const wchar_t*) = &OutputDebugStringW;
	};
	static void Init() {
		Out::A = &OutputDebugStringA;
		Out::W = &OutputDebugStringW;
	}



	std::string GetLogPath(const std::string& subfolder = "", std::string name = "");
	FILE* OpenFile(const std::string& path);
	void  BackupLog(const std::string& logPath, uint32_t max_size = 1024 * 512);
	std::string GetHeader(void* hModule = nullptr, const char* buildtime = "BuildTime: " __DATE__ ", " __TIME__);

	class LogStream {
		std::ostringstream oss_;
		const char* file_;
		const int line_;
	public:
		LogStream(char level, const char* f = nullptr, int l = 0);
		~LogStream();

		inline std::ostream &stream() { return oss_;}
		inline std::string str() { return oss_.str(); }
	};
	
	void DebugPrintf(const char* file, int line, const char* format, ...);
	void DebugPrintf(const char* file, int line, const wchar_t* format, ...);

#ifdef _WIN32
	class win32_category : public std::error_category
	{
	public:
		virtual const char* name() const noexcept {
			return "win32err";
		}
		virtual std::string message(int ev) const
		{
			std::array<char,1024> buf;
			int n = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
				ev,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				buf.data(), (DWORD)buf.size(),
				NULL);
			if (n > 2) {
				sprintf_s(&buf[n - 2], 32, (ev > 0 ? "(%d)": "(0x%x)"), ev);// trim \r\n
			} 
			else {
				HRESULT hr = ev;
				const auto WCODE_HRESULT_FIRST = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200);
				const auto WCODE_HRESULT_LAST = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF + 1, 0) - 1;
				WORD wCode = (hr >= WCODE_HRESULT_FIRST && hr <= WCODE_HRESULT_LAST)
					? WORD(hr - WCODE_HRESULT_FIRST) : 0;
				if (wCode != 0) {
					sprintf_s(buf.data(), buf.size(), "IDispatch error #%d", (int)wCode);
				}
				else {
					sprintf_s(buf.data(), buf.size(), "Unknown error 0x%0lX", hr);
				}
			}

			return buf.data();
		};
	};
#endif // _WIN32


	void enter_scope(const char* lbl);
	void leave_scope(const char* lbl);
	struct FnScope {
		const char* _str;
		FnScope(const char* s) : _str(s) { enter_scope(_str); }
		~FnScope() { leave_scope(_str); }
	};

	class AssertStream
	{
		uint32_t en_;
		std::unique_ptr<LogStream> log_;
	public:
		AssertStream(const char* f, int l, uint32_t en = 0) : en_(en), log_(new LogStream('E', f, l)) {
			createDumpfile(f, l);
			if (en_ != 0)
			{
				log_->stream()
					//<< "errno." << en_ << "/0x" << std::hex << en_
					<< ": " << std::error_code(en_, error_category()).message() << '\n';
			}

		}
		~AssertStream();
		std::ostream& stream() { return log_->stream(); }
	private:
		void createDumpfile(const char* file, int line);
	};


	class ErrnoStream : public LogStream
	{
	public:
		ErrnoStream(const char* f, int l, uint32_t en = ::GetLastError()) : LogStream('E', f, l) {
			stream()
				//<< fmt::csprintf((en >= 0x80000000) ? "en=0x%x, " : "en=%d, ", en)
				<< std::error_code(en, error_category()).message() << "\n\t";
		}
	};


	//
	// reduct too many same log.
	//
	using namespace std::chrono_literals;
	class Loggable
	{
	public:
		bool operator()(const std::string& newLog)
		{
			if (_msg == newLog)
			{
				static std::array<std::chrono::milliseconds, 8> _timeset = { 100ms, 300ms, 1000ms,3000ms,5000ms,30000ms,60000ms,300000ms };
				if (std::chrono::steady_clock::now() - _tick < _timeset[_timeout_index])
					return false;

				if (++_timeout_index == _timeset.size())
					_timeout_index = uint32_t(_timeset.size() - 1);
				_tick = std::chrono::steady_clock::now();
				return true;
			}

			// new message.
			_tick = std::chrono::steady_clock::now();
			_msg = newLog;
			_timeout_index = 0;
			return true;
		}

		bool operator()(uint32_t error_code)
		{
			return this->operator()(std::error_code(error_code, error_category()).message());
		}

		bool operator()(const std::exception& e)
		{
			return this->operator()(e.what());
		}
	private:
		std::chrono::steady_clock::time_point _tick;
		uint32_t _timeout_index = 0;
		std::string _msg;
	};
} // klog


//
// EXPECT(op, expected) or EXPECT(op_boolable)
// EXPECT_NOT(op, not_expected)
//
#define EXPECT(op, ...)                   klog::expect_(#op, __LINE__, (op), ##__VA_ARGS__)
#define EXPECT_NOT(op, not_expected)      klog::expect_not_(#op, __LINE__, (op), not_expected)
namespace klog {
    template<typename T>
    inline T expect_(const char* pos, int line, const T value, const T& expected)
    {
        if (value != expected) {
			klog::ErrnoStream(pos, line).stream()
				<< "expect "<< expected <<", but " << value;
        }
        return value;
    }

    template<typename T>
    inline T expect_(const char* pos, int line, T value )
    {
        if (!value) {
			klog::ErrnoStream(pos, line).stream() << "Failed";
        }
        return value;
    }


    template<typename T>
    inline T expect_not_(const char* pos, int line, T value, const T& not_expected)
    {
        if (value == not_expected) {
            klog::ErrnoStream(pos, line).stream() << "Not expect value=" << not_expected;
        }
        return value;
    }

} // klog


inline std::string ToUtf8(const std::wstring_view& wstr)
{
	if (wstr.length() == 0) return "";
	size_t len;
	std::string buf(wstr.length() * 4 + 16, 0);
#ifdef _WIN32
	len = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, buf.data(), (uint32_t)buf.length(), NULL, NULL);
#else
	wcstombs_s(&len, s.data(), s.size(), msg.data(), msg.length()); // i including \0 char.
#endif
	Assert(len > 0);
	buf.resize(len - 1);
	return buf;
}

inline std::string ToMbcs(const std::wstring_view& wstr)
{
	if (wstr.length() == 0) return "";
	size_t len;
	std::string buf(wstr.length() * 4 + 16, 0);
#ifdef _WIN32
	len = ::WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, buf.data(), (uint32_t)buf.length(), NULL, NULL);
#else
	wcstombs_s(&len, s.data(), s.size(), msg.data(), msg.length()); // i including \0 char.
#endif
	Assert(len > 0);
	buf.resize(len - 1);
	return buf;
}

