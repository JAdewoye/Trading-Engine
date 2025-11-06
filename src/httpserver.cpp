//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "httpserver.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <functional> 
#include <utility>  
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include "TradeQueue.h"


namespace asio  = boost::asio;
using tcp       = asio::ip::tcp;
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
void
HttpServer::run()
{
    try {
    
        std::cout << "HTTP Server started on port " << HttpServer::port_ << "\n";

        while (!get_stop_requested()) {

            // Create a socket that will be moved into new threads
            tcp::socket socket(HttpServer::io_context_);
            std::cout << "HTTP Server: Waiting for new connection...\n";

            // Wait for a new client
            HttpServer::acceptor_.accept(socket);

            try {
                std::thread client_thread(
                    &HttpServer::handle_connection, 
                    this, 
                    std::move(socket)
                );
                // Detach the thread to allow independent execution
                client_thread.detach(); 
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
HttpServer::handle_connection(boost::asio::ip::tcp::socket socket)
{
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    beast::flat_buffer buffer;
    http::request<http::string_body> req;

    try {
        // Read HTTP request
        http::read(socket, buffer, req);

        // Only allow POST to /webhook
        if (req.method() == http::verb::post && req.target() == "/webhook") {
            // Parse JSON payload
            json::value payload = json::parse(req.body());
            std::string symbol = json::value_to<std::string>(payload.at("symbol"));
            std::string side = json::value_to<std::string>(payload.at("side"));
            double price = json::value_to<double>(payload.at("price"));
            std::cout << "Request to " << side << " " << symbol << " at $" << price << "\n";

            // Get current timestamp
            const auto timestamp = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count()
            );

            // Enqueue trade
            queue_.pushBack(timestamp, symbol, side, price);


            // Send OK response
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"status":"success","message":"Webhook received"})";
            res.prepare_payload();
            http::write(socket, res);
        } else {
            // Invalid endpoint
            http::response<http::string_body> res{http::status::not_found, req.version()};
            res.body() = "Endpoint not found";
            res.prepare_payload();
            http::write(socket, res);
        }

        socket.shutdown(tcp::socket::shutdown_send);
    }
    catch (const std::exception& e) {
        std::cerr << "Error in handle_connection: " << e.what() << std::endl;
    }
}