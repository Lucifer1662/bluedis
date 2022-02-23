#pragma once
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <unordered_set>
using boost::asio::ip::tcp;


template<typename tcp_connection>
class tcp_server
{
    tcp::acceptor acceptor_;
    boost::asio::io_service& io_service;
    std::unordered_set<std::shared_ptr<tcp_connection>> connections;

    

public:
    tcp_server(boost::asio::io_service& io_service, int port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        io_service(io_service)
    {}

    void start_accept()
    {
        auto new_connection = std::make_shared<tcp_connection>(io_service);

        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&tcp_server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

    void stop() {
        for (auto& connection : connections) {
            connection->stop();
        }
    }
    std::unordered_set<std::shared_ptr<tcp_connection>>& getConnections() {
        return this->connections;
    };

private:
    void handle_accept(std::shared_ptr<tcp_connection> new_connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "accepted" << std::endl;
            new_connection->socket().local_endpoint();
            connections.insert(new_connection);
            new_connection->onStop.push_back([=]() {
                connections.erase(new_connection);
            });
            new_connection->start();
            start_accept();
        }
        else {
            std::cout << "error accpeting " << error << std::endl;
        }
    }

};