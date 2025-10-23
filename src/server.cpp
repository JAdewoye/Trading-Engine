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
void 
Server::start() 
{
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
        // already running
        return;
    }

    // reset stop flag
    stop_requested_.store(false);

    // Launch thread to run the server
    thread_ = std::thread(&Server::thread_main, this);
}
//----------------------------------------------------------------------------------
void 
Server::thread_main() 
{
    try {
        run();
    } catch (const std::exception& ex) {
        std::cerr << "[" << name_ << "] Uncaught exception in run(): " << ex.what() << '\n';
    } catch (...) {
        std::cerr << "[" << name_ << "] Unknown exception in run()\n";
    }

    running_.store(false);

    // Notify any waiters that we've stopped
    cv_.notify_all();
}
//----------------------------------------------------------------------------------
void
Server::stop() noexcept 
{
    stop_requested_.store(true);
    cv_.notify_all();
}
//----------------------------------------------------------------------------------
void
Server::join() 
{
    if (thread_.joinable()) {
        thread_.join();
    }
}
//----------------------------------------------------------------------------------
bool 
Server::get_stop_requested() const noexcept 
{
    return stop_requested_.load();
}
//----------------------------------------------------------------------------------
bool 
Server::is_running() const noexcept 
{
    return running_.load();
}
//----------------------------------------------------------------------------------
/* Helper: wait for either stop requested or timeout (ms). Returns true if stop requested. */
bool 
Server::wait_for_stop(long milliseconds)
{
    std::unique_lock lock(mtx_);
    
    if (stop_requested_.load()){
        return true;
    }
   
    // Wait indefinitely if milliseconds <= 0    
    if (milliseconds <= 0) {
        cv_.wait(lock, [this]{ return stop_requested_.load(); });
        return true;
    }

    auto timeout = std::chrono::milliseconds(milliseconds);
    cv_.wait_for(lock, timeout, [this]{ return stop_requested_.load(); });
    return stop_requested_.load();
}
//----------------------------------------------------------------------------------
void 
Server::notify_stopped() noexcept 
{
    std::lock_guard lock(mtx_);
    running_.store(false);
    cv_.notify_all();
}
//----------------------------------------------------------------------------------
// End File
//----------------------------------------------------------------------------------