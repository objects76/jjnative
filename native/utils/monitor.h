#pragma once

#include <thread>


#include "logger.h"
#include "interruptable_sleep.h"
#include "staging.h"

#include <memory>


class monitor {
	interruptable_sleep sleep;
	std::thread thread;
public:
	~monitor() { stop(); }

	monitor(const std::string name, std::function<void()> func, int interval_ms)
	{
		thread = std::thread([=] {
			klog::FnScope s(name.c_str());
			try {
				while (true) {
					func();
					if (sleep.interrupted(interval_ms)) break;
				}
			}
			catch (const std::exception& e) {
				LOGE << name << ':' << e.what();
			}
			});
	}

	void stop() {
		sleep.interrupt();
		if (!timed_join(thread))
			LOGE << "thread join timeout.";
	}


	static void test() {
		using namespace std::chrono_literals;



		auto test1 = [&] {
			struct FOBJ {
				int seed = 0;
				elapsed tick;
				const char* label = "monitor label arg";
				void check() {
					auto t = tick.get_and_update();
					LOGI << t.count() << "ms: " << label << ", " << seed++;
				}
			} fobj;

			std::function<void()> handler = std::bind(&FOBJ::check, &fobj);

			monitor m("test1", handler, 1000);
			std::this_thread::sleep_for(5s);
		};

		auto test2 = [] {
			const char* name = "[TEST]";
			int seed = 0;
			elapsed tick;
			const char* label = "monitor label arg";
			auto check = [](auto tick, int seed, const char* label)->void {
				LOGI << '[' << typeid(tick).name() << ']';
				auto t = tick.get_and_update();
				LOGI << t.count() << "ms: " << label << ", " << seed++;
			};

			std::function<void()> handler = std::bind(check, std::ref(tick), seed, label);
			monitor m("test2", handler, 1000);

			std::this_thread::sleep_for(5s);
		};

		auto test3 = [] {

			auto check2 = [](auto tick, int seed, const char* label)->void {
				LOGI << '[' << typeid(tick).name() << ']';
				auto t = tick->get_and_update();
				LOGI << t.count() << "ms: " << label << ", " << seed++;
			};

			auto tick2 = std::make_shared<elapsed>();
			const char* name = "[TEST]";
			const char* label = "monitor label arg";
			int seed = 0;
			std::function<void()> handler = std::bind(check2, tick2, seed, label);
			monitor m("test3", handler, 1000);

			std::this_thread::sleep_for(5s);
		};


		auto test4 = [] {

			auto check2 = [](auto tick, int seed, const char* label)->void {
				//LOGI << '[' << typeid(tick).name() << ']';
				auto t = tick->get_and_update();
				LOGI << t.count() << "ms: " << label << ", " << seed++;
			};

			const char* label = "monitor label arg";
			int seed = 0;
			std::function<void()> handler = std::bind(check2, std::make_shared<elapsed>(), seed, label);
			monitor m("test4", handler, 1000);

			std::this_thread::sleep_for(5s);
		};

		test4();
	}
};