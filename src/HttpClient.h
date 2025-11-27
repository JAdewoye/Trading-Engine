#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "Trading-Engine.h"
//----------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class HttpClient {
public:
    struct OrderResponse {
        std::string order_id{};
        std::string custom_order_id{};
        std::string code{};
        std::string msg{};
        std::string request_time{};
    };

    HttpClient(const std::string& api_key, const std::string& api_secret)
        : api_key_(api_key), api_secret_(api_secret) {}
    ~HttpClient() = default;
    OrderResponse placeOrder(const std::string& symbol, const std::string& side, double price, double quantity);


private:

    std::string api_key_;
    std::string api_secret_;
    std::string base_url_ = "https://api.bitget.com";
};

