#include "Value.h"

std::string valueSerialize(const Value& value) {
	return std::visit([&](const auto& obj) { return obj.serialize(); }, value);
}

std::string valueToString(const Value& value) {
	return std::visit([&](const auto& obj) { return obj.toString(); }, value);
}
