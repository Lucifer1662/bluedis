#pragma once
#include <unordered_map>
#include "Value.h"
#include <functional>
#include <optional>
#include "DB.h"



struct Commands {
	std::unordered_map<std::string, std::function<Value(const Array&, std::function<void()>& cancel)>> commands;

	Value handle(const Value& value) {
		std::function<void()> stopTransaction;
		return handle(value, stopTransaction);
	}

	Value handle(const Value& value, std::function<void()>& stopTransaction) {
		if (is_variant_type<Array>(value)) {
			auto arr = std::get<Array>(value);
			if (arr.elements.size() > 0) {
				auto cmdName = valueToString(arr.elements.front());
				std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(),
					[](unsigned char c) { return std::tolower(c); });
				auto command = commands.find(cmdName);
				if (command != commands.end()) {
					return command->second(arr, stopTransaction);
				}
			}
		}
		return Error("No command");
	}

	template<typename T1, typename T2> 
	auto insert_or_assign(T1&& t1, T2&& t2) {
		return commands.insert_or_assign(std::forward<T1>(t1), std::forward<T2>(t2));
	}


};

class CommandFactory
{
	std::optional<Commands> commands;

public:
	static CommandFactory& getInstance();

	Commands& create();
};

