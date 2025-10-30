//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include "TradeQueue.h"
#include <stdexcept>
#include <iostream>
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
TradeQueue::TradeQueue(const std::size_t capacity): buffer_(capacity + 1), front_(0), back_(0), capacity_(capacity){}
//----------------------------------------------------------------------------------
bool
TradeQueue::pushBack(uint64_t timestamp,const std::string& symbol, const std::string& side, double price)
{
    size_t current_back = back_.load(std::memory_order_relaxed);
    size_t next_back = (current_back + 1) % (capacity_ + 1);

    if (next_back == front_.load(std::memory_order_acquire)) {
        return false; // Queue is full
    }

    buffer_[current_back] = Trade{timestamp,symbol, side, price};

    back_.store(next_back, std::memory_order_release);

    std::cout << "Trade added to queue: " << side << " " << symbol << " at $" << price << "\n";

    return true;
}
//----------------------------------------------------------------------------------
bool
TradeQueue::popFront(Trade& result)
{
    size_t current_front = front_.load(std::memory_order_relaxed);
    size_t next_front = (current_front + 1) % (capacity_ + 1);

    if (current_front == back_.load(std::memory_order_acquire)) {
        return false; // Queue is empty
    }

    result = buffer_[current_front];

    front_.store(next_front, std::memory_order_release);
    return true;
}
//----------------------------------------------------------------------------------
size_t
TradeQueue::getSize()
{
    size_t current_back = back_.load(std::memory_order_acquire);
    size_t current_front = front_.load(std::memory_order_acquire);

    if (current_back >= current_front) {
        return current_back - current_front;
    } else {
        return (capacity_ + current_back) - current_front;
    }
}
//----------------------------------------------------------------------------------