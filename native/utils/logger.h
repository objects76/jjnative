#pragma once
#include <memory>
#include <iostream>
#include <chrono>
#include <array>
#include <string>
#include <string_view>

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cstdarg>
#include <filesystem>

#include <chrono>
#include <ctime>

#define LOGV klog::LogStream('V', __basename__, __LINE__).stream()
#define LOGD klog::LogStream('D', __basename__, __LINE__).stream()
#define LOGI klog::LogStream('I', __basename__, __LINE__).stream()
#define LOGW klog::LogStream('W', __basename__, __LINE__).stream()
#define LOGE klog::LogStream('E', __basename__, __LINE__).stream()
#define LOGV0 klog::LogStream('V', nullptr, 0).stream()
#define LOGD0 klog::LogStream('D', nullptr, 0).stream()
#define LOGI0 klog::LogStream('I', nullptr, 0).stream()
#define LOGW0 klog::LogStream('W', nullptr, 0).stream()
#define LOGE0 klog::LogStream('E', nullptr, 0).stream()

#define __basename__ ([] { constexpr auto x = klog::filename(__FILE__); return x; }())
#define Assert(c) \
	if (!(c))     \
	klog::AssertStream(__basename__, __LINE__, GetLastError()).stream() << "Assert: " #c << '\n'

namespace fmt
{
	inline std::string csprintf(const char *fmt, ...)
	{
		std::array<char, 4096> buf;
		std::va_list args;
		va_start(args, fmt);
		int n = std::vsnprintf(buf.data(), buf.size() - 1, fmt, args);
		va_end(args);

		return std::string(buf.data(), n);
	}

} // namespace fmt

struct tm2 : std::tm
{
	int tm_millisec;
};

inline tm2 localtime2()
{
	using namespace std::chrono;
	using namespace std::chrono_literals;
	system_clock::time_point now = system_clock::now();
	system_clock::duration tp = now.time_since_epoch();

	tp -= duration_cast<seconds>(tp);

	time_t tt = system_clock::to_time_t(now);

	tm2 t;
	*((tm *)&t) = *std::localtime(&tt);
	t.tm_millisec = int(tp / 1ms);
	t.tm_mon += 1;
	t.tm_year += 1900;

	return t;
}

class elapsed
{
	using clock = std::chrono::steady_clock;
	clock::time_point tp = clock::now();

public:
	std::chrono::milliseconds get()
	{
		return std::chrono::duration_cast<std::chrono::duration<uint32_t, std::milli>>(clock::now() - tp);
		//return std::chrono::duration_cast<std::chrono::milliseconds>(tp - old);
	}

	void update()
	{
		tp = clock::now();
	}

	auto get_and_update()
	{
		auto n = get();
		update();
		return n;
	}
};

//
// impl.
//
#ifndef kTag
#define kTag "    "
#endif

#ifndef TOSTR
#define TOSTR2(x) #x
#define TOSTR(x) TOSTR2(x)
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
inline int gettid()
{
	return ::GetCurrentThreadId();
}
inline int getPid() { return ::GetCurrentProcessId(); }

#else
#include <errno.h>
#undef __stdcall
#define __stdcall															// null
#define OutputDebugStringA [](const char *msg) { std::fputs(msg, stdout); } // NL added automatically.
#define GetLastError() (errno)
#endif

namespace klog
{
	struct Out
	{
		inline static void(__stdcall *A)(const char *) = OutputDebugStringA;
	};

	constexpr const char *eos(const char *str)
	{
		return (*str != 0) ? eos(str + 1) : str;
	}
	constexpr bool is_path(const char *str)
	{
		return (*str == std::filesystem::path::preferred_separator) ? true : (*str ? is_path(str + 1) : false);
	}
	constexpr const char *basename(const char *str)
	{
		return (*str == std::filesystem::path::preferred_separator) ? (str + 1) : basename(str - 1);
	}
	constexpr const char *filename(const char *str)
	{
		return is_path(str) ? basename(eos(str)) : str;
	}

	class LogStream
	{
		std::ostringstream oss_;
		const char *file_;
		const int line_;

	public:
		LogStream(char level, const char *f = nullptr, int l = 0) : file_(f), line_(l)
		{
			char buffer[128];
			auto t = localtime2();
			snprintf(buffer, 128, "%c.%-5.5s  %-8d %02u:%02u:%02u.%03u   ", level, kTag, gettid(), t.tm_hour, t.tm_min, t.tm_sec, t.tm_millisec);

			oss_ << buffer;
		}

		~LogStream()
		{
			if (file_)
				oss_ << " at " << file_ << ':' << line_;
			oss_ << '\n';

			Out::A(oss_.str().c_str());
		}

		inline std::ostream &stream() { return oss_; }
		inline std::string str() { return oss_.str(); }
	};

#ifdef _WIN32
	class error_category : public std::error_category
	{
	public:
		virtual const char *name() const noexcept
		{
			return "win32err";
		}
		virtual std::string message(int ev) const
		{
			std::array<char, 1024> buf;
			int n = ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
									 ev,
									 MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
									 buf.data(), (DWORD)buf.size(),
									 NULL);
			if (n > 2)
			{
				sprintf_s(&buf[n - 2], 32, (ev > 0 ? "(%d)" : "(0x%x)"), ev); // trim \r\n
			}
			else
			{
				HRESULT hr = ev;
				const auto WCODE_HRESULT_FIRST = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200);
				const auto WCODE_HRESULT_LAST = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF + 1, 0) - 1;
				WORD wCode = (hr >= WCODE_HRESULT_FIRST && hr <= WCODE_HRESULT_LAST)
								 ? WORD(hr - WCODE_HRESULT_FIRST)
								 : 0;
				if (wCode != 0)
				{
					sprintf_s(buf.data(), buf.size(), "IDispatch error #%d", (int)wCode);
				}
				else
				{
					sprintf_s(buf.data(), buf.size(), "Unknown error 0x%0lX", hr);
				}
			}

			return buf.data();
		};
	};
#else
	struct error_category : public std::system_category()
	{
	}
#endif // _WIN32

	struct FnScope
	{
		const char *_str;
		FnScope(const char *s) : _str(s) { LOGD0 << "{ " << _str; }
		~FnScope() { LOGD0 << "} " << _str; }
	};

	class AssertStream
	{
		uint32_t en_;
		std::unique_ptr<LogStream> log_;

	public:
		AssertStream(const char *f, int l, uint32_t en = 0) : en_(en), log_(new LogStream('E', f, l))
		{
			if (en_ != 0)
			{
				log_->stream()
					<< ": " << std::error_code(en_, error_category()).message() << '\n';
			}
		}
#pragma warning(disable : 4722)
		~AssertStream()
		{
			log_.reset(); // flush
			std::exit(-1);
		}
#pragma warning(default : 4722)
		std::ostream &stream()
		{
			return log_->stream();
		}
	};

	class ErrnoStream : public LogStream
	{
	public:
		ErrnoStream(const char *f, int l, uint32_t en = ::GetLastError()) : LogStream('E', f, l)
		{
			stream()
				<< std::error_code(en, klog::error_category()).message() << "\n\t";
		}
	};

	inline FILE *fopen(const std::string_view path)
	{
		FILE *fp = ::fopen(path.data(), "wt");
		Assert(fp) << path;
		Assert(0 == std::setvbuf(fp, nullptr, _IONBF, 0)); // set nobuffer.
		return fp;
	}

} // namespace klog

inline std::string WideToUtf8(const std::wstring_view &wstr)
{
	if (wstr.length() == 0)
		return "";
	size_t len;
	std::string buf(wstr.length() * 4 + 16, 0);
#ifdef _WIN32
	len = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, buf.data(), (uint32_t)buf.length(), NULL, NULL);
	len -= 1;
#else
	len = std::wcstombs(buf.data(), wstr.data(), wstr.length()); // i including \0 char.
#endif
	Assert(len > 0);
	buf.resize(len);
	return buf;
}

inline std::string WideToAnsi(const std::wstring_view &wstr)
{
	if (wstr.length() == 0)
		return "";
	size_t len;
	std::string buf(wstr.length() * 4 + 16, 0);
#ifdef _WIN32
	len = ::WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, buf.data(), (uint32_t)buf.length(), NULL, NULL);
	len -= 1;
#else
	len = std::wcstombs(buf.data(), wstr.data(), wstr.length());
#endif
	Assert(len > 0);
	buf.resize(len);
	return buf;
}
