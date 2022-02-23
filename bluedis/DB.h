#pragma once
//#ifdef _WIN32
//#include <Windows.h>
//#else
//#include <unistd.h>
//#endif

#include "Value.h"
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <concurrent_unordered_map.h>
#include <concurrent_priority_queue.h>
#include <concurrent_queue.h>
#include <list>
#include <functional>
#include <future>
#include <chrono>
#include "list.h"
#include "string.h"
#include "value_ptr.h"
#include "LoggerAOF.h"



class DB
{
	using Values = concurrency::concurrent_unordered_map<std::string, value_ptr>;
	using val_it = Values::iterator;
	struct value_iterator {
		val_it it;
		timestamp expirey;

		value_iterator(val_it it, timestamp expirey) :it(it), expirey(expirey) {}

		bool operator<(const value_iterator& rhs) const {
			return this->expirey < rhs.expirey;
		}
		value_ptr& val() { return it->second; }

		const std::string& key() { return it->first; }

	};

	std::shared_mutex lock;
	concurrency::concurrent_queue<val_it> dels;
	Values values;
	concurrency::concurrent_priority_queue<value_iterator> expiries;
	std::optional<timestamp> lowestTimeStamp;
	std::thread workerThread;
	bool workerRunning = true;
	const int workerDelay = 2000;//ms

	DB() : workerThread([this]() {this->workerTask(); })
	{
	}


	void workerTask() {
		while (workerRunning) {
			expireKeys();
			deleteBulk();
			//Sleep(workerDelay);
		}
	}

public:

	static DB& getInstance() {
		static DB instance;
		return instance;
	}

	Value get(std::string& str) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(str);
		if (it != values.end()) {
			auto& val = it->second;
			return val.get();
		}
		return Array();
	}

	template<typename Func>
	void set(const std::string& key, const std::string& str, Func&& onDone) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.set(str, std::forward<Func>(onDone));
		}
		else {
			values.insert({ key, value_ptr(std::make_shared<string_value>(str)) });
		}
	}
	template<typename It, typename Func>
	void hset(const std::string& key, It&& begin, It&& end, Func&& onDone) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.hset(std::forward<It>(begin), std::forward<It>(end), std::forward<Func>(onDone));
		}
		else {
			onDone();
			values.insert({ key, value_ptr(std::make_shared<hash_value>(std::forward<It>(begin), std::forward<It>(end))) });
		}
	}

	Value hget(const std::string& key, const std::string& hash_key) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.hget(hash_key);
		}
		else {
			return Array();
		}
	}

	Value hvals(const std::string& key) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.hvals();
		}
		else {
			return Array();
		}
	}

	Value hgetall(const std::string& key) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.hgetall();
		}
		else {
			return Array();
		}
	}


	template<typename It, typename Func>
	Value rpush(const std::string& key, It&& begin, It&& end, Func&& onDone) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			auto& val = it->second;
			return it->second.rpush(begin, end, key, std::forward<Func>(onDone));
		}
		else {
			auto size = Integer(end - begin);
			values.insert({ key, value_ptr(std::make_shared<list_value>(begin, end)) });
			return size;
		}
	}

	template<typename It>
	void deleteListIfEmpty(It& list) {
		if (list->second.lempty())
			delIt(list);

	}

	template<typename It, typename Func>
	Value brpop(It&& begin, It&& end, std::function<void()>& cancel, Func&& onDone) {
		{
			std::lock_guard<std::shared_mutex> lg(lock);

			auto it = begin;
			for (; it != end; it++) {
				auto key = valueToString(*it);

				auto it = values.find(key);
				if (it != values.end()) {
					auto strOpt = it->second.rpop(std::forward<Func>(onDone));
					if (strOpt)
						return Array({
							BulkString(key),
							BulkString(*strOpt)
							});

				}
			}
		}

		auto promise_list = std::make_shared<std::promise<std::pair<std::string, std::string>>>();
		auto future_list = promise_list->get_future();
		cancel = [&]() {
			promise_list->set_exception(std::exception_ptr());
		};


		std::vector<list_value::Queue::iterator> subscribers;
		std::vector<Values::iterator> lists;



		subscribers.reserve(end - begin);

		{
			std::lock_guard<std::shared_mutex> lg(lock);
			auto it = begin;
			for (; it != end; it++) {
				auto key = valueToString(*it);

				auto it = values.find(key);
				if (it != values.end()) {
					subscribers.push_back(it->second.rpop_sub(promise_list));
					lists.push_back(it);
				}
				else {
					auto l = value_ptr(std::make_shared<list_value>());
					subscribers.push_back(l.rpop_sub(promise_list));
					lists.push_back(values.insert({ key,  l }).first);
				}
			}
		}

		auto res = future_list.get();
		if(future_list.valid())
			onDone();

		auto list_it = lists.begin();
		for (auto& sub : subscribers) {
			(*list_it)->second.unsubr(sub);
			deleteListIfEmpty(*list_it);
			list_it++;
		}



		return Array({
			BulkString(res.first),
			BulkString(res.second),
			});
	}

	Value exists(const std::string& key) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.exists();
		}
		else {
			return Integer(0);
		}
	}

	template<typename Func>
	Value setExpirey(std::string& key, timestamp timestamp, Func&& onDone) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			auto retOpt = it->second.setExpirey(timestamp, std::forward<Func>(onDone));
			if (retOpt)
				return *retOpt;

			expiries.push({ it,timestamp, });
			return Integer(1);
		}
		return Array();
	}

	bool del(const std::string& key) {
		auto it = values.find(key);
		if (it == values.end()) {
			return false;
		}
		else {
			delIt(it);
			return true;
		}
	}

	template<typename It>
	void delIt(It& it) {
		if (it->second.del()) {
			dels.push(it);
		}
	}


	template<typename Func>
	Value add(const std::string& key, long amount, Func&& onDone) {
		std::lock_guard<std::shared_mutex> lg(lock);
		auto it = values.find(key);
		if (it != values.end()) {
			return it->second.add(amount, std::forward<Func>(onDone));
		}
		else {
			onDone();
			values.insert({ key, value_ptr(std::make_shared<string_value>("0")) });
			return Integer(0);
		}
	}

	void deleteBulk() {
		if (!dels.empty()) {
			lock.lock();
			auto val = values.end();
			while (true) {
				auto p = dels.try_pop(val);
				if (!p) {
					break;
				}
				values.unsafe_erase(val);
			}
			lock.unlock();
		}
	}

	void expireKeys() {
		auto end = values.end();
		value_iterator val(end, 0);
		timestamp now = timestamp::now();

		while (true) {
			if (expiries.try_pop(val)) {
				val.val()->lock();
				if (!val.val()->timestamp || *val.val()->timestamp < now) {
					dels.push(val.it);
					val.val()->unlock();
				}
				else {
					expiries.push(val);
					val.val()->unlock();
					return;
				}
			}
			else {
				return;
			}
		}
	}

	~DB() {
		workerRunning = false;
		workerThread.join();
	}

};

