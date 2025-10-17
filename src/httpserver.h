#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "server.h"
#include <boost/asio.hpp>
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class HttpServer : public Server {
protected:
    void run() override;

    void handle_connection(boost::asio::ip::tcp::socket socket);
};
