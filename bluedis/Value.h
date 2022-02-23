#pragma once
#include <variant>
#include <string>
#include <vector>
#include <istream>


enum class ValueType : char {
	Integer = ':',
	Error = '-',
	SimpleString = '+',
	BulkString = '$',
	Arrays = '*'

};

struct Integer {

	long val;
	Integer(long val) :val(val) {}
	Integer() = default;

	std::string toString() const {
		return  std::to_string(val);
	}

	std::string serialize() const {
		return (char)ValueType::Integer + toString() + "\r\n";
	}
};

struct Error {
	std::string error;
	Error(const char* str, size_t len) : error(str, len) {}
	Error(std::string&& str) : error(str) {}

	std::string toString() const {
		return error;
	}

	std::string serialize() const {
		return (char)ValueType::Error + toString() +"\r\n";
	}
};

struct BulkString {
	std::string str;
	BulkString(const char* str, size_t len) : str(str, len) {}
	BulkString(std::istream& stream, size_t len) : str(len, ' ') {
		stream.read(str.data(), len);
	}
	BulkString(std::string&& str) : str(str) {}
	BulkString(const std::string& str) : str(str) {}

	std::string toString() const {
		return str;
	}

	std::string serialize() const {
		return (char)ValueType::BulkString + std::to_string(str.length()) +
			"\r\n" + toString() + "\r\n";
	}
};

struct NilBulkString {
	std::string toString() const {
		return "nil";
	}

	std::string serialize() const {
		return (char)ValueType::BulkString + "-1\r\n";
	}

};

struct SimpleString {
	std::string str;
	SimpleString(const char* str, size_t len) : str(str, len) {}
	SimpleString(std::string&& str) : str(str) {}

	std::string toString() const {
		return str;
	}

	std::string serialize() const {
		return (char)ValueType::SimpleString + toString() + "\r\n";
	}

};


struct Array;

using Value = std::variant<Array, Integer, Error, BulkString, NilBulkString, SimpleString>;

std::string valueToString(const Value& value);
std::string valueSerialize(const Value& value);

template<typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t variant_index() {
	if constexpr (index == std::variant_size_v<VariantType>) {
		return index;
	}
	else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
		return index;
	}
	else {
		return variant_index<VariantType, T, index + 1>();
	}
}
template<typename T, typename VariantType>
constexpr bool is_variant_type(const VariantType& value) {
	return value.index() == variant_index<VariantType, Array>();
}

struct Array {
	std::vector<Value> elements;

	Array() = default;
	Array(const Array&) = default;
	Array(Array&&) = default;
	Array(std::vector<Value>&& elements) :elements(elements) {}
	Array(const std::vector<Value>& elements) :elements(elements) {}
	Array(std::initializer_list<Value>&& elements) :elements(elements) {}

	std::string toString() const {
		std::string str;
		for (const auto& value : elements) {
			str += valueToString(value) + "\r\n";
		}
		return str;
	}

	std::string serialize() const {
		std::string str = (char)ValueType::Arrays + std::to_string(elements.size()) + "\r\n";
		for (const auto& value : elements) {
			str += valueSerialize(value);
		}
		return str;
	}
};


