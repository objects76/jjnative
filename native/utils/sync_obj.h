#pragma once
#include <mutex>

#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <set>
#include "logger.h"

template <typename T> 
class sync_obj
{
private:
	mutable T t;
	mutable std::mutex m;

	sync_obj(const sync_obj&) = delete;
	sync_obj& operator=(const sync_obj&) = delete;
public:
	const auto get() const {
		struct proxy {
			T* const p;
			std::mutex* m;
			proxy(T* const p_, std::mutex* m_) : p(p_), m(m_) { m->lock(); }
			~proxy() { m->unlock(); }
			T* operator->() { return p; }
			const T* operator->() const { return p; }
		};
		return proxy(&t, &m);
	}
	auto operator -> () { return get(); }

	template< typename ...CtorArgs>
	sync_obj(CtorArgs ...args) : t(args...) {}


	static void test() {
		using namespace std::chrono_literals;
		sync_obj<std::vector<int>> obj;


		std::vector<std::thread> threads;
		for (int seed = 0; seed < 100000; seed += 100)
			threads.emplace_back(std::thread([&](int seed) {
				for (int i = 0; i < 100; ++i) {
					std::this_thread::sleep_for(1ms);
					obj->push_back(seed + i);
				};
			}, seed));
		for (auto& t : threads) t.join();


		{
			std::set<int> check;
			int n = 0;

			while (!obj->empty())
			{
				auto proxy = obj.get();
				//LOGI << proxy->back() << std::endl;

				if (check.count(proxy->back()) > 0)
				{
					std::cerr << "dup: " << proxy->back() << std::endl;
				}
				else {
					++n;
				}
				check.insert(proxy->back());
				proxy->pop_back();
			}

			LOGI << "total op:" << n << ", check:" << check.size() << std::endl;
		}
	}


};

