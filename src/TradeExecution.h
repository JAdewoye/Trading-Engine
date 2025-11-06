#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeQueue.h"
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
//----------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class TradeExecutionPool {
public:
    TradeExecutionPool(std::size_t num_workers, TradeQueue& trade_queue);
    ~TradeExecutionPool();

private:
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_requested_ = false;
    TradeQueue& trade_queue_;

    void worker_loop(void);
    void execute_trade(const Trade& trade);
    void log_trade_execution(const Trade& trade);
};

