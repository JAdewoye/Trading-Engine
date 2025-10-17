//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "httpserver.h"
#include <boost/asio.hpp>
#include <iostream>

namespace asio  = boost::asio;
using tcp       = asio::ip::tcp;
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
void
HttpServer::run(){
    try {
        asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));

        while (!get_stop_requested()) {
            tcp::socket socket(io_service);

            // Wait for a new client
            acceptor.accept(socket);

            try {
                // Handle the client connection
                handle_connection(std::move(socket));
            }
            catch (const std::exception& e) {
                std::cerr << "Connection handling error: " << e.what() << "\n";
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
    }
}
//----------------------------------------------------------------------------------
void
HttpServer::handle_connection(tcp::socket socket){
    
}