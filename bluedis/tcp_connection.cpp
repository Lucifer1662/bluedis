#include "tcp_connection.h"


tcp_connection::pointer tcp_connection::create(boost::asio::io_service& io_service)
{
    auto tcp = std::make_shared<tcp_connection>(io_service);
    tcp->self = tcp;
    return tcp;
}

tcp::socket& tcp_connection::socket()
{
    return (tcp::socket&)stream_.socket();
}

void tcp_connection::start()
{
   
}

tcp::iostream& tcp_connection::stream()
{
    return stream_;
}

tcp_connection::tcp_connection(boost::asio::io_service& io_service)
    : stream_(std::move(tcp::socket(io_service))), io_service(io_service)
{

}