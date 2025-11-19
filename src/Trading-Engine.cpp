//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "httpserver.h"
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
        Queue<Trade> trade_queue(5);
        TradeDatabase trade_db("dbname=trading_engine user=trader password=secret host=localhost port=5432");
        TradeExecutionPool execution_pool(5, trade_queue, trade_db);
        HttpServer server("http_server", HTTP_DEFAULT_PORT, trade_queue, 5);

        
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
