#pragma once
#include "base_value.h"
#include <charconv>

struct string_value : base_value {
	std::string str;

	string_value() : base_value(ValType::String) {}
	string_value(std::string&& str) :str(std::move(str)), base_value(ValType::String) {}
	string_value(const std::string& str) :str(str), base_value(ValType::String) {}
	string_value(const string_value& s) : str(s.str), base_value(ValType::String) {}
	string_value(string_value&& s) : str(std::move(s.str)), base_value(ValType::String) {}


	string_value& operator=(const string_value&) = default;


	std::optional<std::string> get() {
		if (isDeleted) {
			return {};
		}
		else {
			return this->str;
		}
	}

	void set(const std::string& str) {
		this->str = str;
		isDeleted = false;
	}


	std::optional<long> add(long amount) {
		std::optional<long> val = 0;
		if (isDeleted) {
			str = "0";
			isDeleted = false;
		}
		else {
			if (std::from_chars(str.data(), str.data() + str.size(), *val).ec == std::errc()) {
				*val += amount;
				str = std::to_string(*val);
			}
			else {
				val = {};
			}
		}
		return val;
	}

	bool exists() {
		return !isDeleted;
	}

	void del() {}
};


