#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include <string>
#include <memory>
#include "TradeQueue.h"
//#include <pqxx/pqxx>
//----------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class TradeDatabase {
public:
    TradeDatabase(const std::string& connection_string);
    ~TradeDatabase();

    bool saveTrade(const Trade& trade);
private:
    //std::unique_ptr<pqxx::connection> connection_;
    void ensureTableExists();
};

