#pragma once
#include "base_value.h"
#include <unordered_map>
#include "Value.h"

class hash_value : public base_value {
	using map = std::unordered_map<std::string, std::string>;
	map elements;

public:

	hash_value() : base_value(ValType::HashMap) {}
	hash_value(const hash_value& s) : base_value(ValType::HashMap) {}
	hash_value(hash_value&& s) : base_value(ValType::HashMap) {}
	

	hash_value(const map& s) : elements(elements), base_value(ValType::HashMap) {}
	hash_value(map&& s) : elements(elements), base_value(ValType::HashMap) {}
	
	template<typename It>
	hash_value(It&& begin, It&& end) : base_value(ValType::HashMap) {
		set(std::forward<It>(begin), std::forward<It>(end));
	}

	template<typename It>
	void set(It begin, It end) {
		while(begin != end) {
			elements.insert_or_assign(valueToString(*begin), valueToString(*(begin + 1)));
			begin++;
			begin++;
		}
	}

	std::optional<std::string> get(const std::string& key) {
		auto it = elements.find(key);
		if (it != elements.end()) {
			return it->second;
		}
		return {};
	}

	const map& elems() const {
		return this->elements;
	}

};

