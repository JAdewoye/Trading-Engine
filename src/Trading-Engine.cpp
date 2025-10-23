//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "httpserver.h"
#include "Trading-Engine.h"
#include <iostream>

//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------

int main()
{
	std::cout << "Trading Engine Starting...\n";
    try {
        HttpServer server("http_server", HTTP_DEFAULT_PORT);
        server.start();

        std::cout << "Server is running.\n";
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
