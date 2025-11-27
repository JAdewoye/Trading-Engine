//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "HttpServer.h"
#include "Trading-Engine.h"
#include "Queue.h"
#include "TradeExecution.h"
#include "TradeDatabase.h"
#include <iostream>

//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------

int main()
{
    std::cout << "Trading Engine Starting...\n";

    try {
        std::string port, db_name, db_user, db_password;

        std::cout << "Port: ";
        std::cin >> port;

        std::cout << "DB Name: ";
        std::cin >> db_name;

        std::cout << "User: ";
        std::cin >> db_user;

        std::cout << "Password: ";
        std::cin >> db_password;

        std::cout << "[MAIN] Building connection string...\n";
        std::string connection_str =
            "dbname=" + db_name +
            " user=" + db_user +
            " password=" + db_password +
            " host=localhost port=" + port;

        std::unique_ptr<TradeDatabase> trade_db;
        try {
            std::cout << "[MAIN] Constructing TradeDatabase...\n";
            trade_db = std::make_unique<TradeDatabase>(connection_str);
            std::cout << "[MAIN] TradeDatabase OK.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] TradeDatabase failed: " << e.what() << "\n";
            throw;
        }

        std::unique_ptr<Queue<Trade>> trade_queue;
        try {
            std::cout << "[MAIN] Creating Trade Queue...\n";
            trade_queue = std::make_unique<Queue<Trade>>(TRADE_ENTRY_QUEUE_SIZE);
            std::cout << "[MAIN] Queue OK.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] Queue failed: " << e.what() << "\n";
            throw;
        }

        std::unique_ptr<TradeExecutionPool> execution_pool;
        try {
            std::cout << "[MAIN] Creating ExecutionPool...\n";
            execution_pool = std::make_unique<TradeExecutionPool>(
                TRADE_WORKER_COUNT,
                *trade_queue,
                *trade_db
            );
            std::cout << "[MAIN] ExecutionPool OK.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] ExecutionPool failed: " << e.what() << "\n";
            throw;
        }

        std::unique_ptr<HttpServer> server;
        try {
            std::cout << "[MAIN] Creating HttpServer...\n";
            server = std::make_unique<HttpServer>(
                "http_server",
                HTTP_DEFAULT_PORT,
                *trade_queue,
                TRADE_WORKER_COUNT
            );
            std::cout << "[MAIN] HttpServer OK.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] HttpServer failed: " << e.what() << "\n";
            throw;
        }

        try {
            std::cout << "[MAIN] Starting DB...\n";
            trade_db->start();
            std::cout << "[MAIN] DB started.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] trade_db.start(): " << e.what() << "\n";
            throw;
        }

        try {
            std::cout << "[MAIN] Starting server...\n";
            server->start();
            std::cout << "[MAIN] Server started.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] server.start(): " << e.what() << "\n";
            throw;
        }

        std::cout << "[MAIN] Server is running. Press Enter to stop.\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();

        server->stop();
        server->join();
    }
    catch (const std::exception& e) {
        std::cerr << "\n[FATAL] Uncaught exception: " << e.what() << "\n";
    }
    catch (...) {
        std::cerr << "\n[FATAL] Unknown non-std exception!\n";
    }

    return 0;

}
