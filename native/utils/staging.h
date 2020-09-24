#pragma once

#include <thread>
#include <future>
#include <chrono>
#include <array>
#include <string_view>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

using namespace std::chrono_literals;

// thread::join() with timeout.
inline bool timed_join(std::thread &thread, std::chrono::seconds timeout_sec = 5s)
{
	if (thread.joinable())
	{
		auto future = std::async(std::launch::async, &std::thread::join, &thread);
		if (future.wait_for(timeout_sec) == std::future_status::timeout)
		{
			return false;
		}
	}

	return true;
}

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

inline auto localtime2()
{
	using clock = std::chrono::system_clock;
	const auto now = clock::now();
	clock::duration tp = now.time_since_epoch();

	tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

	time_t tt = clock::to_time_t(now);

	struct tmex : std::tm
	{
		int tm_millisec;
	} t;

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
	auto get()
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

#ifndef kTag
#define kTag "    "
#endif

#ifndef TOSTR
#define TOSTR2(x) #x
#define TOSTR(x) TOSTR2(x)
#endif
