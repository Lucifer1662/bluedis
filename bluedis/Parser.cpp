#include "Parser.h"

long parseInt(std::string_view& str) {
	long val;
	size_t clrIndex = str.find_first_of('\r');
	std::from_chars(str.data(), str.data() + clrIndex, val);
	str = str.substr(clrIndex + 2);
	return val;
}

std::string getString(std::istream& stream) {
	std::string str;
	while (true) {
		char c = stream.get();
		if (c == '\r')
			break;
		str += c;
	}
	stream.get();
	return str;
}

long parseInt(std::istream& stream) {
	long val;
	auto str = getString(stream);
	std::from_chars(str.data(), str.data() + str.size(), val);
	return val;
}

Value parse(std::string_view& str) {
	char type = str[0];
	str = str.substr(1);
	switch (type)
	{
	case (char)ValueType::Integer: {
		return Integer(parseInt(str));
	}

	case (char)ValueType::Error: {
		size_t clrIndex = str.find_first_of('\r');
		auto error = Error(str.data(), clrIndex);
		str = str.substr(clrIndex + 2);
		return error;
	}

	case (char)ValueType::BulkString: {
		size_t length = parseInt(str);
		if (length == -1) {
			return NilBulkString();
		}
		else {
			size_t clrIndex = length;
			auto bulkStr = BulkString(str.data(), clrIndex);
			str = str.substr(clrIndex + 2);
			return bulkStr;
		}
	}

	case (char)ValueType::SimpleString: {
		size_t clrIndex = str.find_first_of('\r');
		auto error = SimpleString(str.data(), clrIndex);
		str = str.substr(clrIndex + 2);
		return error;
	}
	case (char)ValueType::Arrays: {
		auto length = parseInt(str);
		Array array;
		array.elements.reserve(length);
		for (size_t i = 0; i < length; i++) {
			array.elements.emplace_back(parse(str));
		}
		return array;
	}

	default:
		break;
	}
}

Value parse(std::istream& stream) {
	char type = stream.get();
	switch (type)
	{
	case (char)ValueType::Integer: {
		return Integer(parseInt(stream));
	}

	case (char)ValueType::Error: {
		return Error(getString(stream));
	}

	case (char)ValueType::BulkString: {
		size_t length = parseInt(stream);
		if (length == -1) {
			return NilBulkString();
		}
		else {
			auto bulkStr = BulkString(stream, length);
			stream.get();
			stream.get();
			return bulkStr;
		}
	}

	case (char)ValueType::SimpleString: {
		return SimpleString(getString(stream));
	}
	case (char)ValueType::Arrays: {
		auto length = parseInt(stream);
		Array array;
		array.elements.reserve(length);
		for (size_t i = 0; i < length; i++) {
			array.elements.emplace_back(parse(stream));
		}
		return array;
	}

	default:
		break;
	}
}
