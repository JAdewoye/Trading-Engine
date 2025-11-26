#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "Queue.h"
#include "TradeDatabase.h"
#include "Trading-Engine.h"
#include "HttpClient.h"
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
    TradeExecutionPool(std::size_t num_workers, Queue<Trade>& trade_queue, TradeDatabase& trade_db);
    ~TradeExecutionPool();

private:
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_requested_ = false;
    Queue<Trade>& trade_queue_;
    TradeDatabase& trade_db_;

    void workerLoop(void);
    void executeTrade(const Trade& cell);
    void logTradeExecution(const Trade& cell, const HttpClient::OrderResponse& response);
};

