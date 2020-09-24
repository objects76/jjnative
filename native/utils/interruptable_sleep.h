#pragma once


#include <condition_variable>
#include <mutex>
#include <functional>

#include <chrono>
#include <iostream>
#include <thread>

class interruptable_sleep
{
	std::condition_variable cv;
	std::mutex m;
	bool signaled = false;

public:
	bool interrupted(unsigned int timeout_ms) {
		std::unique_lock<std::mutex> lock(m);
		return cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&] {return signaled; }) == true;
	}

	void interrupt() {
		std::lock_guard<std::mutex> lock(m);
		signaled = true;
		cv.notify_all();
	}
};

