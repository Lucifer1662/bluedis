#pragma once
#include <atomic>
#include <optional>
#include "timestamp.h"

struct spin_lock {
	mutable std::atomic<bool> lock_ = { false };

	void lock() const {
		for (;;) {
			if (!lock_.exchange(true, std::memory_order_acquire)) {
				break;
			}
			while (lock_.load(std::memory_order_relaxed));
		}
	}

	void unlock() const { lock_.store(false, std::memory_order_release); }
};


enum class ValType : char {
	String,
	List,
	Set,
	HashMap,
	Bit
};

class base_value : public spin_lock {
	ValType type_;
public:
	bool isDeleted = false;
	std::optional<timestamp> timestamp;

	base_value(ValType type_) :type_(type_) {};
	base_value(const base_value& s) : isDeleted(s.isDeleted), timestamp(s.timestamp) {}
	base_value(base_value&& s) : isDeleted(s.isDeleted), timestamp(s.timestamp) {}


	bool operator<(const base_value& rhs) {
		lock();
		rhs.lock();
		bool val = *timestamp < *rhs.timestamp;
		rhs.unlock();
		unlock();
		return val;
	}

	inline void del() {
		isDeleted = true;
	}

	inline ValType type() {
		return this->type_;
	}
};

