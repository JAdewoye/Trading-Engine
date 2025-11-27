#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "Queue.h"
#include "Trading-Engine.h"
#include "HttpClient.h"
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
    struct DBConfig {
        std::string host;
        std::string port;
        std::string dbname;
        std::string user;
        std::string password;
    };

    struct DBentry{
        Trade trade_ref;
        HttpClient::OrderResponse response_ref;
    };

    TradeDatabase(const std::string& connection_string) : running_(false), connected_(false), connection_string_(connection_string) {}
    ~TradeDatabase(){
        stop();
        if (connection_ && connection_->is_open()) {
            std::cout << "Database connection closed.\n";
        }
    }
    
    bool saveTrade(const DBentry& db_entry);
    void workerLoop();
    void start();
    void stop();
    bool isConnected();
    Queue<DBentry> log_queue_{TRADE_LOG_QUEUE_SIZE};

private:
    void tryConnect();

    std::unique_ptr<pqxx::connection> connection_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    std::atomic<bool> connected_;
    std::string connection_string_;
};

