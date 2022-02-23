#pragma once
#include "tcp_connection.h"
#include "Parser.h"
#include <thread>
#include <memory>
#include <variant>
#include "CommandFactory.h"
#include <vector>
#include "LoggerAOF.h"



class client_connection : public tcp_connection
{
	boost::asio::streambuf readBuffer;
	bool running = false;
	std::string current;
	size_t offset = 0;
	Parser parser;
	Commands& commands;
	std::function<void()> stopTransaction;
	AOFLogger_ptr aof_logger;

public:
	client_connection(boost::asio::io_service& io_service) : tcp_connection(io_service), 
		parser(offset), commands(CommandFactory::getInstance().create()),
		aof_logger(AOFLogger_Factory::getInstance().create())
	{}

	void f(boost::system::error_code e) {
		std::cout << e << std::endl;
		if (!e.failed()) {
			async_wait_for_read([this](auto e) {this->f(e); });
		}
		else {
			stop();
		}
	}

	void start() {
			
		async_wait_for_read([this](auto e) {this->f(e); });

		auto& stream = this->stream();

		running = true;
		while (running) {
			try {
				auto value = parse(stream);
				//aof_logger->log(value);
				auto response = commands.handle(value, stopTransaction);
				stream << valueSerialize(response);
			}
			catch (std::exception e) {
				running = false;
			}
		}

		/*boost::asio::async_read_until(socket(), readBuffer, "\r\n", [=](auto& string) {
		current += string;
		auto valueOpt = parser.parse(string);
		if (valueOpt) {
		handleValue(value);
		current = current.substr(parser.offset);
		}

		std::cout << string << std::endl;
		start();
		});*/

	}


	void stop() {
		this->running = false;
		stopTransaction();
		tcp_connection::stop();
	}


	/*template<typename Handler>
	void async_read_string(Handler&& handler) {
		boost::asio::async_read_until(socket(), this->readBuffer, '\n',
			[=](auto error, auto transferSize) {
				std::string buf;
				buf.resize(transferSize);
				this->readBuffer.sgetn(&buf[0], buf.size());
				buf.find_first_of('\n');

				std::cout << buf << std::endl;
			});
	}*/
};


class client_connection_thread : public client_connection {
	std::unique_ptr<std::thread> thread;

public:
	client_connection_thread(boost::asio::io_service& io_service) : client_connection(io_service) {}

	void start() {
		thread = std::make_unique<std::thread>([this]() {
			this->client_connection::start();
			});
	}

	void stop() {
		client_connection::stop();
	}

	void join() {
		thread->join();
	}
};

