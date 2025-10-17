#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class Server {
public:
    Server(std::string name_ = "Server") noexcept;
    virtual ~Server();

    // Copy Constructor and assignment disabled
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Start the server thread. No-op if already running.
    void start();

    // Request the server to stop. This sets a flag and notifies the thread.
    // Does not block; call join() to wait for thread termination.
    void stop() noexcept;

    // Join the server thread (blocks until thread exits). Safe to call if not started.
    void join();

    // Returns true if stop has been requested.
    bool get_stop_requested() const noexcept;

    // Returns true if the server thread is running.
    bool is_running() const noexcept;

    // Human-readable server name for logs.
    const std::string& get_name() const noexcept { return name; }

protected:
    // This is executed inside the thread created by start().
    // Implementation should periodically check stopRequested() and return quickly on shutdown.
    virtual void run() = 0;

    virtual void handle_connection(/* socket */) = 0;

    // Helper: wait for either stop requested or timeout (ms). Returns true if stop requested.
    bool wait_for_stop(long milliseconds);

    void Server::notify_stopped() noexcept;

private:
    void thread_main(); // internal entrypoint for the thread

    std::string name;
    std::thread thread;
    std::atomic<bool> running;
    std::atomic<bool> stop_requested;
    mutable std::mutex mtx;
    std::condition_variable cv;
};