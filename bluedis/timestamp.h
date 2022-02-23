#pragma once
#include <string>
#include <chrono>

struct timestamp {
	size_t value;
	timestamp(size_t time) : value(time) {}
	timestamp(const std::string& time) : value(std::atol(time.c_str())) {}
	timestamp(const timestamp& time) = default;
	timestamp(timestamp&& time) = default;
	timestamp& operator=(const timestamp& b) = default;


	timestamp operator +(timestamp b) const { return value + b.value; }
	timestamp operator -(timestamp b) const { return value - b.value; }
	bool operator >(timestamp b) const { return value > b.value; }
	bool operator >=(timestamp b) const { return value >= b.value; }
	bool operator <(timestamp b) const { return value < b.value; }
	bool operator <=(timestamp b) const { return value <= b.value; }
	bool operator ==(timestamp b) const { return value == b.value; }

	template<typename T>
	timestamp operator *(T b) { return value * b; }
	template<typename T>
	timestamp operator /(T b) { return value / b; }

	static timestamp now() {
		return std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}
};

