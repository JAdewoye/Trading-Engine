#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "Queue.h"
#include "Trading-Engine.h"
#include <string>
#include <memory>
#include <pqxx/pqxx>
//----------------------------------------------------------------------------------
// struct Definitions
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class TradeDatabase {
public:
    TradeDatabase(const std::string& connection_string);
    ~TradeDatabase();
    
    bool saveTrade(const Trade& trade);

    Queue<Trade> log_queue_{1000};
private:
    void ensureTableExists();
    
    std::unique_ptr<pqxx::connection> connection_;
};

