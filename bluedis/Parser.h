#pragma once
#include <memory>
#include <string>
#include "Value.h"
#include <optional>
#include <charconv>
#include <istream>


struct Parser {
	char type = 0;
	size_t& offset;
	size_t start = 0;
	size_t arraySizeLeft = 0;
	std::shared_ptr<Parser> stack;
	Value value;
	int bulkStringSize = -1;
	Parser(size_t& offset) :offset(offset) {}

	long parseInt(std::string_view str) {
		long val;
		std::from_chars(str.data(), str.data() + str.size(), val);
		return val;
	}

	std::optional<Value> parse(std::string_view str) {
		for (; offset < str.size(); ) {
			if (stack) {
				while (arraySizeLeft > 0 && offset < str.size()) {
					auto valueOpt = stack->parse(str);
					if (valueOpt) {
						std::get<Array>(value).elements.push_back(*valueOpt);
						arraySizeLeft--;
						stack = nullptr;
						if (arraySizeLeft > 0)
							stack = std::make_shared<Parser>(offset);
					}
				}
				if (arraySizeLeft == 0) {
					return value;
				}
			}
			else if (type == 0) {
				type = str[offset];
				start = offset + 1;
				offset++;
			}
			else if (str[offset] == '\n') {
				switch (type)
				{
				case (char)ValueType::Integer: {
					offset++;
					return Integer(parseInt(str.substr(start, offset - 2 - start)));
				}

				case (char)ValueType::Error: {
					offset++;
					return Error(str.data() + start, offset - 2 - start);
				}
				case (char)ValueType::SimpleString: {
					offset++;
					return SimpleString(str.data() + start, offset - 2 - start);
				}
				case (char)ValueType::Arrays: {
					arraySizeLeft = parseInt(str.substr(start, offset - 1 - start));
					stack = std::make_shared<Parser>(offset);
					offset++;
					break;
				}
				case (char)ValueType::BulkString: {
					if (bulkStringSize == -1) {
						size_t length = parseInt(str.substr(start, offset - 1 - start));
						if (length == -1) {
							offset++;
							return NilBulkString();
						}
						else {
							start = offset + 1;
							offset += length + 2;
							bulkStringSize = length;
						}
						break;
					}
					else {
						offset++;
						return BulkString(str.data() + start, bulkStringSize);
					}
				}
				}
			}
			else {
				offset++;
			}
		}
		return {};

	}
};



long parseInt(std::string_view& str);

std::string getString(std::istream& stream);

long parseInt(std::istream& stream);



Value parse(std::string_view& str);

Value parse(std::istream& stream);