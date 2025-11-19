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
#include "Queue.h"


namespace asio  = boost::asio;
using tcp       = asio::ip::tcp;
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
HttpServer::HttpServer(const std::string& name, unsigned short port, Queue<Trade>& queue, size_t num_workers)
    : Server(name), 
      port_(port),
      queue_(queue),
      num_workers_(num_workers),
      io_context_(),
      acceptor_(io_context_, tcp::endpoint(boost::asio::ip::tcp::v4(), port_)) 
{}
//----------------------------------------------------------------------------------
HttpServer::~HttpServer()
{
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
//----------------------------------------------------------------------------------
void
HttpServer::run()
{
    try {
    
        std::cout << "HTTP Server started on port " << HttpServer::port_ << ". We are now read to trade!\n";

        auto guard = boost::asio::make_work_guard(io_context_);

        start_accept();

        // Launch worker threads to run the io_context
        for (size_t i = 0; i < num_workers_; ++i) {
            workers_.emplace_back([this]{
                 this->io_context_.run(); 
            });
        }

        // Wait for shutdown (your loop can do other things)
        while (!get_stop_requested()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // Stop io_context_ and let the work_guard fall out of scope
        io_context_.stop();

        // Join threads or run the io_context in the main thread
        for (auto& worker : workers_) {
            worker.join();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
    }
}
//----------------------------------------------------------------------------------
void HttpServer::start_accept() {
    auto socket = std::make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) mutable {
        if (!ec) {
            // Handle the connection in a separate function
            std::cout << "Accepted new connection\n";
            handle_connection(std::move(*socket));
        } else {
            std::cout << "Accept error: " << ec.message() << "\n";
            // Log the error
        }
        start_accept(); // Continue accepting new connections
    });
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

            Trade trade{timestamp, symbol, side, price};
            Cell<Trade> trade_entry{false, trade};

            // Enqueue trade
            if (queue_.pushBack(trade_entry)) {
                // Send OK response
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"status":"success","message":"Webhook received"})";
                res.prepare_payload();
                http::write(socket, res);
            }else{
                // Send OK response
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"status":"success","message":"Webhook received - Trade Fialed"})";
                res.prepare_payload();
                http::write(socket, res);
            }


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