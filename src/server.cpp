//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "server.h"
#include <iostream>
#include <chrono>
using namespace std::chrono_literals;
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
Server::Server(std::string name_) noexcept{
    name = name_;
    running = false;
    stop_requested = false;
}
//----------------------------------------------------------------------------------
Server::~Server() {
    // Attempt graceful shutdown if still running
    stop();
    join();
}
//----------------------------------------------------------------------------------
void 
Server::start() {
    bool expected = false;
    if (!running.compare_exchange_strong(expected, true)) {
        // already running
        return;
    }

    // reset stop flag
    stop_requested.store(false);

    // Launch thread to run the server
    thread = std::thread(&Server::thread_main, this);
}
//----------------------------------------------------------------------------------
void 
Server::thread_main() {
    try {
        run();
    } catch (const std::exception& ex) {
        std::cerr << "[" << name << "] Uncaught exception in run(): " << ex.what() << '\n';
    } catch (...) {
        std::cerr << "[" << name << "] Unknown exception in run()\n";
    }

    running.store(false);

    // Notify any waiters that we've stopped
    cv.notify_all();
}
//----------------------------------------------------------------------------------
void
 Server::stop() noexcept {
    stop_requested.store(true);
    cv.notify_all();
}
//----------------------------------------------------------------------------------
void
Server::join() {
    if (thread.joinable()) {
        thread.join();
    }
}
//----------------------------------------------------------------------------------
bool 
Server::get_stop_requested() const noexcept {
    return stop_requested.load();
}
//----------------------------------------------------------------------------------
bool 
Server::is_running() const noexcept {
    return running.load();
}
//----------------------------------------------------------------------------------
/* Helper: wait for either stop requested or timeout (ms). Returns true if stop requested. */
bool 
Server::wait_for_stop(long milliseconds) {
    std::unique_lock lock(mtx);
    
    if (stop_requested.load()){
        return true;
    }
   
    // Wait indefinitely if milliseconds <= 0    
    if (milliseconds <= 0) {
        cv.wait(lock, [this]{ return stop_requested.load(); });
        return true;
    }

    auto timeout = std::chrono::milliseconds(milliseconds);
    cv.wait_for(lock, timeout, [this]{ return stop_requested.load(); });
    return stop_requested.load();
}
//----------------------------------------------------------------------------------
void 
Server::notify_stopped() noexcept {
    std::lock_guard lock(mtx);
    running.store(false);
    cv.notify_all();
}
//----------------------------------------------------------------------------------
// End File
//----------------------------------------------------------------------------------