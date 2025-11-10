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
    HttpServer(const std::string& name, unsigned short port, TradeQueue& queue, size_t num_workers);
    ~HttpServer();
    void run() override;
    void handle_connection(tcp::socket socket);
    void start_accept();

private:

    unsigned short port_;
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    TradeQueue& queue_;
    std::vector<std::thread> workers_;
    size_t num_workers_;
};

