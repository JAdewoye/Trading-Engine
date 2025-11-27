//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeDatabase.h"
#include <iostream>
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
void
TradeDatabase::workerLoop()
{
    Trade trade;
    HttpClient::OrderResponse response;
    DBentry db_entry{trade, response};
    while (running_.load(std::memory_order_acquire)) {
        if (!connected_.load(std::memory_order_acquire)) {
                tryConnect();
                std::this_thread::sleep_for(std::chrono::seconds(100));
                continue;
        }

        if (log_queue_.popFront(db_entry)){
            saveTrade(db_entry);
        } else{
            std::this_thread::yield();
        }
    }
}
//----------------------------------------------------------------------------------
bool
TradeDatabase::saveTrade(const DBentry& db_entry)
{
    if (!connection_) { 
        std::cerr << "Error saving trade: Database connection is not established (connection_ is null).\n";
        return false;
    }
    
    try {
        pqxx::work tx(*connection_);

        // TODO add trade but also response into the database
        tx.exec_params(
            "INSERT INTO trades (symbol, side, price, timestamp) VALUES ($1, $2, $3, $4)",
            db_entry.trade_ref.symbol,
            db_entry.trade_ref.side,
            db_entry.trade_ref.price,
            db_entry.trade_ref.timestamp
        );

        tx.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving trade: " << e.what() << "\n";
        return false;
    }
}
//----------------------------------------------------------------------------------
void
TradeDatabase::start()
{
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
        return;
    }

    worker_thread_ = std::thread(&TradeDatabase::workerLoop, this);
}
//----------------------------------------------------------------------------------
void
TradeDatabase::stop()
{
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false)) {
        return;
    }

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}
//----------------------------------------------------------------------------------
bool
TradeDatabase::isConnected()
{
    return connected_.load(std::memory_order_acquire);
}
//----------------------------------------------------------------------------------
void
TradeDatabase::tryConnect()
{
    try {
        connection_ = std::make_unique<pqxx::connection>(connection_string_);
        if (connection_->is_open()) {
            connected_.store(true, std::memory_order_release);
            std::cout << "Connected to database successfully.\n";
        } else {
            connected_.store(false, std::memory_order_release);
            std::cerr << "Failed to open database connection.\n";
        }
    } catch (const std::exception& e) {
        connected_.store(false, std::memory_order_release);
        std::cerr << "Database connection error: " << e.what() << "\n";
    }
}