#pragma once
#include <memory>
#include "base_value.h"
#include "Value.h"
#include <thread>
#include <mutex>
#include "string.h"
#include "list.h"
#include "hash_value.h"

struct value_ptr : public std::shared_ptr<base_value> {

	value_ptr() = default;
	value_ptr(value_ptr&&) = default;
	value_ptr(const value_ptr&) = default;
	value_ptr(std::shared_ptr<base_value>&& v) :std::shared_ptr<base_value>(v) {}
	value_ptr(const std::shared_ptr<base_value>& v) :std::shared_ptr<base_value>(v) {}

	Value get() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::String) {
			auto strOpt = ((string_value&)*val).get();
			if (strOpt)
				return BulkString(*strOpt);
		}

		return Array();

	}

	template<typename Func>
	void set(const std::string& str, Func&& onDone) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::String) {
			((string_value&)*val).set(str);
		}
		onDone();
	}

	
	template<typename It, typename Func>
	void hset(It&& begin, It&& end, Func&& onDone){
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::HashMap) {
			((hash_value&)*val).set(std::forward<It>(begin), std::forward<It>(end));
		}		
		onDone();
	}

	Value hget(const std::string& key) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::HashMap) {
			auto retOpt = ((hash_value&)*val).get(key);
			if (retOpt) {
				return BulkString(*retOpt);
			}
			else {
				return Array();
			}
		}
		return Error("Is not a hash map");
	}

	Value hvals() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::HashMap) {
			auto elems = ((hash_value&)*val).elems();
			Array vals;
			vals.elements.reserve(elems.size());
			for (auto& keypair : elems) {
				vals.elements.emplace_back(keypair.second);
			}
			return vals;
		}
		return Error("Is not a hash map");
	}

	Value hgetall() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::HashMap) {
			auto elems = ((hash_value&)*val).elems();
			Array vals;
			vals.elements.reserve(elems.size());
			for (auto& keypair : elems) {
				vals.elements.emplace_back(keypair.first);
				vals.elements.emplace_back(keypair.second);
			}
			return vals;
		}
		return Error("Is not a hash map");
	}

	template<typename It, typename Func>
	Value rpush(It&& begin, It&& end, const std::string& key, Func&& onDone) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::List) {
			auto value = Integer(((list_value&)*val).rpush(begin, end, key));
			onDone();
			return value;
		}
		else {
			auto error = Error("Is not list");
			onDone();
			return error;
		}
	}

	template<typename Func>
	std::optional<Value> setExpirey(timestamp timestamp, Func&& onDone) {
		std::lock_guard<base_value> lg(**this);
		auto& val = **this;
		if (val.isDeleted) {
			onDone();
			return 0;
		}
		if (val.timestamp && *val.timestamp == timestamp) {
			onDone();
			return 1;
		}
		val.timestamp = timestamp;
		onDone();
	}



	template<typename Func>
	Value add(long amount, Func&& onDone) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::String) {
			auto valOpt = ((string_value&)*val).add(amount);
			if (valOpt) {
				auto integer = Integer(*valOpt);
				onDone();
				return integer;
			}
		}
		onDone();
		return Error("Was not an integer");
	}

	bool del() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;

		bool wasDeleted = val->isDeleted;
		val->isDeleted = true;

		switch (val->type())
		{
		case ValType::String: { ((string_value&)*val).del(); break; }
		case ValType::List: { ((list_value&)*val).del(); break; }

		default:
			return false;
		}
		return wasDeleted;
	}

	template<typename T>
	auto rpop_sub(T&& t) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		return ((list_value&)*val).rpop_sub(std::forward<T>(t));
	}


	template<typename It>
	void unsubr(It&& it) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		((list_value&)*val).rqueue.erase(std::forward<It>(it));
	}

	template<typename Func>
	std::optional<std::string> rpop(Func&& onDone) {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		onDone();
		return ((list_value&)*val).rpop();
	}

	std::optional<size_t> llen() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		if (val->type() == ValType::List) {
			return ((list_value&)*val).len();
		}
		return {};
	}

	bool lempty() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;
		return ((list_value&)*val).empty();
	}

	bool exists() {
		std::lock_guard<base_value> lg(**this);
		auto& val = *this;

		switch (val->type())
		{
		case ValType::String: return ((string_value&)*val).exists();
		case ValType::List: return ((list_value&)*val).exists();

		default:
			return false;
		}
	}

};






