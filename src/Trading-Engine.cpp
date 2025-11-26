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
        std::string port;
        std::string db_name;
        std::string db_user;
        std::string db_password;
        
        std::cout << "Port: ";
        std::cin >> port;
        
        std::cout << "DB Name: ";
        std::cin >> db_name;
        
        std::cout << "User: ";
        std::cin >> db_user;
        
        std::cout << "Password: ";
        std::cin >> db_password;
        
        std::string connection_str = "dbname=" + db_name + " user=" + db_user + " password=" + db_password + " host=localhost port=" + port;
        TradeDatabase trade_db(connection_str);

        Queue<Trade> trade_queue(TRADE_ENTRY_QUEUE_SIZE);
        TradeExecutionPool execution_pool(TRADE_WORKER_COUNT, trade_queue, trade_db);
        HttpServer server("http_server", HTTP_DEFAULT_PORT, trade_queue, TRADE_WORKER_COUNT);
        
        trade_db.start();
        server.start();
        std::cin.get();
        server.stop();
        server.join();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
	return 0;
}
