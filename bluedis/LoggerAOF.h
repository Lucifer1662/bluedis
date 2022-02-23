#pragma once
#include "Value.h"
#include <unordered_set>
#include <deque>
#include <mutex>
#include <fstream>
#include <future>

class LoggerAOF {
protected:  

	static std::unordered_set<std::string> writeOperations;

	virtual void logValue(const Value& value) {};

	void log(const Array& arr) {
		if (arr.elements.size() > 0 && isWriteOperation(valueToString(arr.elements.front()))) {
			logValue(arr);
		}
	}

	bool isWriteOperation(const std::string& str) {
		return writeOperations.find(str) != writeOperations.end();
	}

public:
	void log(const Value& value) {
		if (is_variant_type<Array>(value)) {
			log(std::get<Array>(value));
		}
	}
	

	virtual ~LoggerAOF() = default;
};

class AOF_Interval : public LoggerAOF {
	std::mutex lock;
	std::deque<Value> log_;
	std::promise<bool> promise;
	std::ofstream file;
	size_t seconds;

	std::thread intervalThread = std::thread([this]() {
		auto future = promise.get_future();
		while (true) {

			auto status = future.wait_for(std::chrono::seconds(seconds));
			if (status == std::future_status::ready 
				|| status == std::future_status::deferred)
				break;
		}

	});

	void save() {
		auto lg = std::lock_guard(lock);
		for (auto& value : log_) {
			file << valueSerialize(value);
		}
		file.flush();

	}

public:
	AOF_Interval(std::ofstream&& file) : file(std::move(file)) {}


	virtual void logValue(const Value& value) {
		auto lg = std::lock_guard(lock);
		log_.push_back(value);
	}

	virtual ~AOF_Interval() {
		promise.set_value(false);
	}

};

class AOF_FSync : public LoggerAOF {
	std::mutex lock;
	std::ofstream file;


public:
	AOF_FSync(std::ofstream&& file) : file(std::move(file)) {}

	virtual void logValue(const Value& value) {
		auto lg = std::lock_guard(lock);
		file << valueSerialize(value);
		file.flush();
	}

	
};

enum class AOF_Type {
	None,
	FSync,
	Interval
};

using AOFLogger_ptr = std::shared_ptr<LoggerAOF>;
class AOFLogger_Factory {
	AOFLogger_ptr logger;
	AOF_Type type = AOF_Type::None;
	std::string outputFile;
public:

	static AOFLogger_Factory& getInstance() {
		static AOFLogger_Factory instance;
		return instance;
	}

	void setType(AOF_Type type) {
		this->type = type;
	}

	void setOutputFile(const std::string& outputFile) {
		this->outputFile = outputFile;
	}

	const std::string& getOutputFile() {
		return outputFile;
	}

	AOFLogger_ptr create() {
		if (logger)
			return logger;

		switch (type)
		{
		case AOF_Type::None: return logger = std::make_shared<LoggerAOF>();
		case AOF_Type::FSync: {
			return std::make_shared<AOF_FSync>(std::move(std::ofstream(outputFile, std::ios_base::app)));
		}
		case AOF_Type::Interval: return logger = std::make_shared<AOF_Interval>(
			std::ofstream(outputFile, std::ios_base::app));
		
		default:
			break;
		}
	}
	



};





