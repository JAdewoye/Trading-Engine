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
        std::string order_id = 0;
        std::string custom_order_id = 0;
        std::string code = 0;
        std::string msg = 0;
        std::string request_time = 0;
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

