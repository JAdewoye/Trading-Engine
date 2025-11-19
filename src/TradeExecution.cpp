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
    Cell<Trade> trade_entry;
    while (stop_requested_.load(std::memory_order_acquire) == false) {
        if (trade_queue_.popFront(trade_entry)){
            executeTrade(trade_entry.entry);
            logTradeExecution(trade_entry.entry);
        } else{
            std::this_thread::yield();
        }
    }
}
//----------------------------------------------------------------------------------
void
TradeExecutionPool::executeTrade(const Trade& trade_entry)
{

}
//----------------------------------------------------------------------------------
void
TradeExecutionPool::logTradeExecution(const Trade& trade_entry)
{
    Cell<Trade> trade_cell{true, trade_entry};
    trade_db_.log_queue_.pushBack(trade_cell);
}