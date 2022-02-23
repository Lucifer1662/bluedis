#pragma once
#include <list>
#include "base_value.h"
#include <memory>
#include <future>
#include <charconv>


struct list_value : public base_value {
	std::list<std::string> elements;
	using Queue = std::list<std::shared_ptr<std::promise<std::pair<std::string, std::string>>>>;
	Queue rqueue;
	Queue lqueue;
	list_value() : base_value(ValType::List) {}

	template<typename It>
	list_value(It&& begin, It&& end) : base_value(ValType::List) {
		for (; begin != end; begin++) {
			elements.push_back(valueToString(*begin));
		}
	}
	list_value(const list_value& s) : elements(s.elements), base_value(ValType::List) {}
	list_value(list_value&& s) : elements(std::move(s.elements)), base_value(ValType::List) {}

	list_value& operator=(const list_value&) = default;

	template<typename T>
	auto rpop_sub(T&& t) {
		rqueue.push_back(std::forward<T>(t));
		return --rqueue.end();
	}

	template<typename T>
	size_t lpush(T&& t) {
		elements.push_front(std::forward<T>(t));
		isDeleted = false;
		return elements.size();
	}

	template<typename T>
	size_t rpush(T&& t, const std::string& key) {
		auto it = rqueue.begin();
		while (it != rqueue.end()) {
			try {
				rqueue.front()->set_value({ key, t });
				return elements.size();
			}
			catch (std::exception e) {}
			it++;
		}

		elements.push_back(std::forward<T>(t));
		isDeleted = false;
		return elements.size();
	}

	std::optional<std::string> rpop() {
		if (elements.size() > 0) {
			auto str = elements.back();
			elements.pop_back();
			return str;
		}
		else {
			return {};
		}
	}

	size_t len() {
		return elements.size();
	}

	bool empty() {
		return len() == 0 && rqueue.empty() && lqueue.empty();
	}

	bool exists() {
		return len() > 0;
	}

	void del() {
		elements.clear();
	}

	template<typename It>
	size_t rpush(It&& begin, It&& end, const std::string& key) {
		for (; begin != end; begin++) {
			rpush(valueToString(*begin), key);
		}

		return elements.size();
	}
};

