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