#pragma once
//----------------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------------
#include <iostream>
//----------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------
#define TRADE_LOG_QUEUE_SIZE 1000
#define TRADE_ENTRY_QUEUE_SIZE 100
#define TRADE_WORKER_COUNT 5
#define NEON_CONNECTION_STRING "postgresql://neondb_owner:npg_3z0ftDReMXKn@ep-misty-math-ab7mlr4c-pooler.eu-west-2.aws.neon.tech/Trades?sslmode=require&channel_binding=require"
#define NEON_DB_NAME "trade_entries"
#define BITGET_HOST "api.bitget.com"
#define BITGET_TRADE_ENDPOINT "/api/v2/mix/order/place-order"
#define API_KEY "bg_41e44bfb5bde92eff0db806a4c79bfd8"
#define SECRET_KEY "56f8a4b0117180e226f7018c6a93035f5a0d04b08fe0962ac6bf75939ae09a6f"
#define PASSPHRASE "Blessed1234"
#define USER_PRODUCT_TYPE "USDT-FUTURES"
#define USER_MARGINE_MODE "isolated"
#define USER_MARGIN_COIN "USDT"
#define USER_ORDER_TYPE "limit"


//----------------------------------------------------------------------------------
// Struct Definitions
//----------------------------------------------------------------------------------
struct Trade {
    uint64_t timestamp = 0;
    std::string symbol{};
    std::string side{};
    double price = 0;
    double quantity = 0;
};