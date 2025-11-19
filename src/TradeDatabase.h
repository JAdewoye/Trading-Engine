#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "Queue.h"
#include "Trading-Engine.h"
#include <string>
#include <memory>
#include <pqxx/pqxx>
#include <thread>
#include <atomic>
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
    void workerLoop();
    void start();
    void stop();
    bool isConnected();
    Queue<Trade> log_queue_{TRADE_LOG_QUEUE_SIZE};

private:
    void tryConnect();

    
    std::unique_ptr<pqxx::connection> connection_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    std::atomic<bool> connected_;
    std::string connection_string_;
};

