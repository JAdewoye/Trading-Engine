//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeDatabase.h"
#include <iostream>
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
TradeDatabase::TradeDatabase(const std::string& connection_string) : connection_(std::make_unique<pqxx::connection>(connection_string))
{
    if (connection_ && connection_->is_open()) {
        std::cout << "Connected to database successfully: " << connection_->dbname() << "\n";
        //ensureTableExists();
    } else {
        throw std::runtime_error("Failed to connect to database");
    }
}
//----------------------------------------------------------------------------------
TradeDatabase::~TradeDatabase()
{
    if (connection_ && connection_->is_open()) {
        //connection_->disconnect();
        std::cout << "Database connection closed.\n";
    }
}
//----------------------------------------------------------------------------------
bool
TradeDatabase::saveTrade(const Trade& trade_entry)
{
    try {
        pqxx::work tx(*connection_);

        tx.exec_params(
            "INSERT INTO trades (symbol, side, price, timestamp) VALUES ($1, $2, $3, $4)",
            trade_entry.symbol,
            trade_entry.side,
            trade_entry.price,
            trade_entry.timestamp
        );

        tx.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving trade: " << e.what() << "\n";
        return false;
    }
}
