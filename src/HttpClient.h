#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "Trading-Engine.h"
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;
//----------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class HttpClient {
public:
    struct OrderResponse {
        bool success = false;
        std::string order_id{};
        std::string custom_order_id{};
        std::string code{};
        std::string msg{};
        long long request_time{};
    };

    HttpClient() {}
    ~HttpClient() = default;
    OrderResponse placeOrder(std::string const& body);
    std::string prepareOrderBody(const std::string& symbol, const std::string& side, double price, double quantity);

private:
    std::string calculate_access_sign(const std::string& secret_key, const std::string& message_string);
    OrderResponse sendRequest(http::request<http::string_body>& req);
    void parseResponse(http::response<http::string_body>& res, HttpClient::OrderResponse& response);
};

