//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeExecution.h"
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
TradeExecutionPool::TradeExecutionPool(std::size_t num_workers, Queue<Trade>& trade_queue, TradeDatabase& trade_db) : 
stop_requested_(false), trade_queue_(trade_queue), trade_db_(trade_db)
{
    for (std::size_t i = 0; i < num_workers; ++i) {
        workers_.emplace_back(&TradeExecutionPool::workerLoop, this);
    }
}
//----------------------------------------------------------------------------------
TradeExecutionPool::~TradeExecutionPool()
{
    stop_requested_.store(false, std::memory_order_release);
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
//----------------------------------------------------------------------------------
void 
TradeExecutionPool::workerLoop()
{
    Trade trade_entry;
    while (stop_requested_.load(std::memory_order_acquire) == false) {
        if (trade_queue_.popFront(trade_entry)){
            executeTrade(trade_entry);
        } else{
            std::this_thread::yield();
        }
    }
}
//----------------------------------------------------------------------------------
void
TradeExecutionPool::executeTrade(const Trade& trade_entry)
{
    std::string api_key;
    std::string api_secret;

    HttpClient client(api_key, api_secret);
    HttpClient::OrderResponse response = client.placeOrder(trade_entry.symbol, trade_entry.side, trade_entry.price, trade_entry.quantity);

    if (response.code == "00000") {
        std::cout << "Trade executed: " << trade_entry.side << " " << trade_entry.symbol 
                  << " at $" << trade_entry.price << " Qty: " << trade_entry.quantity 
                  << " Order ID: " << response.order_id << "\n";
    } else {
        // Try again with a short delay to avoid rapid repeated requests
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        response = client.placeOrder(trade_entry.symbol, trade_entry.side, trade_entry.price, trade_entry.quantity);
        if (response.code == "00000") {
            std::cout << "Trade executed on retry: " << trade_entry.side << " " << trade_entry.symbol 
                      << " at $" << trade_entry.price << " Qty: " << trade_entry.quantity 
                      << " Order ID: " << response.order_id << "\n";
            std::cerr << "Failed to execute trade: " << trade_entry.side << " " << trade_entry.symbol 
                      << " at $" << trade_entry.price << " Qty: " << trade_entry.quantity 
                      << " Order ID: " << response.order_id 
                      << " Error: " << response.msg << "\n";
        }

    }

    logTradeExecution(trade_entry, response);
}
//----------------------------------------------------------------------------------
void
TradeExecutionPool::logTradeExecution(const Trade& trade_entry, const HttpClient::OrderResponse& response)
{
    TradeDatabase::DBentry db_entry(trade_entry, response);
    trade_db_.log_queue_.pushBack(std::move(db_entry));
}