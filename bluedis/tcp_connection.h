#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "Event.h"
using boost::asio::ip::tcp;
using error_code = boost::system::error_code;
using endpoint = boost::asio::ip::tcp::endpoint;


class tcp_connection
{
	tcp::iostream stream_;
	std::weak_ptr<tcp_connection> self;
	std::string host;
	int port;


public:
	Event<> onStop;

	boost::asio::io_service& io_service;


	template<typename T>
	T read() {
		T t;
		auto buffer = boost::asio::buffer((void*)&t, sizeof(T));
		boost::asio::read(socket(), buffer, boost::asio::transfer_exactly(sizeof(T)));
		return t;
	}


	template<typename Handler>
	void async_wait_for_read(Handler&& handler) {
		socket().async_wait(boost::asio::ip::tcp::socket::wait_read, handler);
	}

	template<typename Handler>
	void async_wait_for_error(Handler&& handler) {
		socket().async_wait(boost::asio::ip::tcp::socket::wait_error, handler);
	}
	
	void stop() {
		socket().close();
		onStop();
	}

	


	bool write(void* data, int size) {
		boost::system::error_code ec;
		try {
			boost::asio::write(socket(), boost::asio::buffer(data, size), ec);
		}
		catch (std::exception e) {
			onError(ec);
		}
		return !ec.failed();
	}

	bool writeBuf(boost::asio::streambuf& buffer) {
		error_code ec;
		try {
			boost::asio::write(socket(), buffer, ec);
		}
		catch (std::exception e) {
			onError(ec);
		}
		return !ec.failed();
	}

	template<typename T>
	bool write(T object) {
		boost::system::error_code ec;
		try {
			boost::asio::write(socket(), boost::asio::buffer((void*)&object, sizeof(T)), ec);
		}
		catch (std::exception e) {
			onError(ec);
		}
		return !ec.failed();;
	}

	template<>
	bool write<std::string>(std::string t) {
		auto suc1 = tcp_connection::write<size_t>(t.size());
		auto suc2 = tcp_connection::write((void*)t.c_str(), t.size());
		return suc1 && suc2;
	}

	typedef std::shared_ptr<tcp_connection> pointer;
	static pointer create(boost::asio::io_service& io_service);

	tcp::socket& socket();

	void start();

	tcp::iostream& stream();

	//tcp_connection(const tcp_connection&) = default;
	//tcp_connection(tcp_connection&&) = default;
	tcp_connection(boost::asio::io_service& io_service);

	template<typename Handler>
	void connect(Handler&& handler) {
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("127.0.0.1", std::to_string(port));
		auto endpoint = resolver.resolve(query);
		tcp::resolver::iterator end;
		boost::asio::async_connect(socket(), endpoint, handler);
	}

	void connect(std::string host, int port) {
		this->host = host;
		this->port = port;
		connect([&](auto error, auto endpoint) {
			std::cout << endpoint.port() << std::endl;
		});
		
	}



	void onError(error_code& error) {
	
	}


};