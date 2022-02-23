#pragma once
#include <list>
#include <functional>

template<typename... Args>
struct Event : public std::list<std::function<void(Args...)>> {
	std::list<std::function<void(Args...)>> queue;

	void operator()(Args... args) {
		for (auto& func : *this) {
			func(args...);
		}
		
		for (auto& func : queue) {
			func(args...);
		}
		queue.clear();
	}

	template<typename Func>
	void push_to_single(Func&& func) {
		queue.push_back(func);
	}
};

