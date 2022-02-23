#pragma once
#include <optional>
#include <charconv>


template<typename T>
std::optional<T> parse(const char* first, const char* last) {
	std::optional<T> val = 0;

	if (std::from_chars(first, last, *val).ec == std::errc()) {
		return val;
	}
	return {};
}

template<typename T, typename S>
std::optional<T> parse(S&& s) {
	return parse<T>(s.data(), s.data() + s.size());
}