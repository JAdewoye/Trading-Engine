//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeDatabase.h"
#include <iostream>
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
TradeDatabase::TradeDatabase(const std::string& connection_string) :
running_(false), connected_(false), connection_string_(connection_string) {}
//----------------------------------------------------------------------------------
TradeDatabase::~TradeDatabase()
{
    stop();
    if (connection_ && connection_->is_open()) {
        std::cout << "Database connection closed.\n";
    }
}
//----------------------------------------------------------------------------------
void
TradeDatabase::workerLoop()
{
    Cell<Trade> trade_entry;
    while (running_.load(std::memory_order_acquire)) {
        if (!connected_.load(std::memory_order_acquire)) {
                tryConnect();
                std::this_thread::sleep_for(std::chrono::seconds(100));
                continue;
        }

        if (log_queue_.popFront(trade_entry)){
            saveTrade(trade_entry.entry);
        } else{
            std::this_thread::yield();
        }
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