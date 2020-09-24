#pragma once

#include <thread>
#include <future>
#include <chrono>
#include <string_view>
#include <cstdio>

#include "logger.h"

using namespace std::chrono_literals;


// thread::join() with timeout.
inline bool timed_join(std::thread& thread, std::chrono::seconds timeout_sec = 5s)
{
	if (thread.joinable())
	{
		auto future = std::async(std::launch::async, &std::thread::join, &thread);
		if (future.wait_for(timeout_sec) == std::future_status::timeout) {
			return false;
		}
	}

	return true;
}
