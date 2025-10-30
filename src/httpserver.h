#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "server.h"
#include "TradeQueue.h"
#include <boost/asio.hpp>
//----------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------
#define HTTP_DEFAULT_PORT 8000

using tcp = boost::asio::ip::tcp;
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class HttpServer : public Server {
public:
    HttpServer(const std::string& name, unsigned short port)
        : Server(name), port_(port),
          io_context_(),
          acceptor_(io_context_, tcp::endpoint(boost::asio::ip::tcp::v4(), port_)) 
    {}

    void run() override;
    void handle_connection(tcp::socket socket, TradeQueue& tradeQueue);

private:

    unsigned short port_;
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
};

