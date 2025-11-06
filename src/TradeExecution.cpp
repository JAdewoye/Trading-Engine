//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeExecution.h"
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
TradeExecutionPool::TradeExecutionPool(std::size_t num_workers, TradeQueue& trade_queue) : stop_requested_(false), trade_queue_(trade_queue)
{
    for (std::size_t i = 0; i < num_workers; ++i) {
        workers_.emplace_back(&TradeExecutionPool::worker_loop, this);
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
TradeExecutionPool::worker_loop()
{
    Trade trade;
    while (stop_requested_.load(std::memory_order_acquire) == false) {
        if (trade_queue_.popFront(trade)){
            execute_trade(trade);
            log_trade_execution(trade);
        } else{
            std::this_thread::yield();
        }
    }
}
//----------------------------------------------------------------------------------
void
TradeExecutionPool::execute_trade(const Trade& trade)
{

}
//----------------------------------------------------------------------------------
void
TradeExecutionPool::log_trade_execution(const Trade& trade)
{
    std::cout << "Executed trade: " << trade.side << " " << trade.symbol << " at $" << trade.price << "\n";
}