#include "CommandFactory.h"
#include "client_connection.h"
#include "util.h"
#include "LoggerAOF.h"


CommandFactory& CommandFactory::getInstance() {
	static CommandFactory factory;

	return factory;
}

Commands& CommandFactory::create() {
	if (!commands) {
		commands = Commands();
		auto aofLogger = AOFLogger_Factory::getInstance().create();

		commands->insert_or_assign("set", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 3)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);

			DB::getInstance().set(key, valueToString(args.elements[2]), [=]() {
				aofLogger->log((Value)args);
				});
			return Integer(1);
			});

		commands->insert_or_assign("hset", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 4)
				return Error("Not enough args");

			if (args.elements.size() % 2 == 1)
				return Error("Missing key-pair");

			auto key = valueToString(args.elements[1]);

			DB::getInstance().hset(key, args.elements.begin()+2, args.elements.end(), [=]() {
				aofLogger->log((Value)args);
				});
			return Integer(1);
			});

		commands->insert_or_assign("hget", [](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 3)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);
			auto hash_key = valueToString(args.elements[2]);


			return DB::getInstance().hget(key, hash_key);
			});

		commands->insert_or_assign("hvals", [](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 2)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);


			return DB::getInstance().hvals(key);
			});

		commands->insert_or_assign("hgetall", [](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 2)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);

			return DB::getInstance().hgetall(key);
			});

		commands->insert_or_assign("get", [](const Array& args, std::function<void()>& cancel) -> Value {
			auto key = valueToString(args.elements[1]);
			if (args.elements.size() < 2)
				return Error("Not enough args");

			return DB::getInstance().get(key);

			});
		commands->insert_or_assign("del", [](const Array& args, std::function<void()>& cancel) -> Value {
			auto key = valueToString(args.elements[1]);
			if (args.elements.size() < 2)
				return Error("Not enough args");

			return DB::getInstance().del(key);

			});

		commands->insert_or_assign("expire", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 3)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);
			auto expirey = timestamp::now() + timestamp(valueToString(args.elements[2]));

			return DB::getInstance().setExpirey(key, expirey, [=]() {
				aofLogger->log((Value)args);
				});
			});

		commands->insert_or_assign("incr", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 2)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);


			return DB::getInstance().add(key, 1, [=]() {
				aofLogger->log((Value)args);
				});
			});

		commands->insert_or_assign("incrby", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 2)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);
			auto val = parse<long>(valueToString(args.elements[2]));
			if (!val) {
				return Error("arg not a number");
			}

			return DB::getInstance().add(key, *val, [=]() {
				aofLogger->log((Value)args);
				});
			});




		commands->insert_or_assign("rpush", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 3)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);

			return DB::getInstance().rpush(key, args.elements.begin() + 2, args.elements.end(), [=]() {
				aofLogger->log((Value)args);
				});

			});


		commands->insert_or_assign("info", [](const Array& args, std::function<void()>& cancel) -> Value {
			return BulkString("");

			});

		commands->insert_or_assign("brpop", [=](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 2)
				return Error("Not enough args");


			auto key = valueToString(args.elements[1]);

			return DB::getInstance().brpop(args.elements.begin() + 1, args.elements.end(), cancel, 
				[=]() {
				aofLogger->log((Value)args);
				});

			});

		commands->insert_or_assign("exists", [](const Array& args, std::function<void()>& cancel) -> Value {
			if (args.elements.size() < 2)
				return Error("Not enough args");

			auto key = valueToString(args.elements[1]);


			return DB::getInstance().exists(key);

			});
	}
	return *commands;
}
