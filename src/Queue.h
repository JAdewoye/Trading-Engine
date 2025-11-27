#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
//----------------------------------------------------------------------------------
// Struct Definitions
//----------------------------------------------------------------------------------
template<typename T>
struct Cell {
    std::atomic_bool occupied {false};
    T entry;
};
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
template<typename T>
class Queue{
public:
    Queue(const std::size_t capacity);
    ~Queue() = default;
    bool pushBack(T&& entry_ptr);
    bool popFront(T& result_ptr);
    size_t getSize();

private:
    std::vector<Cell<T>> buffer_;
    alignas(64) std::atomic_size_t front_;
    alignas(64) std::atomic_size_t back_;
    const size_t capacity_;
};
//----------------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------------
template<typename T>
Queue<T>::Queue(const std::size_t capacity): buffer_(capacity), front_(0), back_(0), capacity_(capacity){}
//----------------------------------------------------------------------------------
template<typename T> bool
Queue<T>::pushBack(T&& entry)
{
    size_t next_back;
    size_t current_back;

    do {
        current_back = back_.load(std::memory_order_relaxed);
        next_back = (current_back + 1) % (capacity_);

        // Check if the queue is full
        if (next_back == front_.load(std::memory_order_acquire)) {
            return false; 
        }

        // Attempt to claim the next back index
    } while (!back_.compare_exchange_weak(current_back, next_back, std::memory_order_release, std::memory_order_relaxed));

    // Write data after a slot has been successfully reserved
    buffer_[current_back].entry = std::move(entry);
    buffer_[current_back].occupied.store(true, std::memory_order_release);

    return true;
}
//----------------------------------------------------------------------------------
template<typename T> bool
Queue<T>::popFront(T& result)
{
    // Claim the front index using the atomic operation fetch_add to incrmement front index for next pop.
    size_t current_front;
    size_t current_back;
    size_t corrected_front;

    // Check if the queue is empty. If so, we need to rollback the front index increment and return false.
    do{
        current_front = front_.load(std::memory_order_acquire);
        current_back = back_.load(std::memory_order_acquire);
        corrected_front = (current_front + 1) % (capacity_);

        // queue is empty
        if ((current_front == current_back) || (corrected_front >= capacity_)) {
            return false;
        }

    } while (!front_.compare_exchange_weak(current_front, corrected_front, std::memory_order_release, std::memory_order_relaxed));


    while (buffer_[current_front].occupied.load(std::memory_order_acquire) == false) {
        // Busy-wait until the cell is occupied
        std::this_thread::yield();
    }

    // Because we claimed the front index and the queue is not empty, we can safely read the trade
    result = std::move(buffer_[current_front].entry);
    buffer_[current_front].occupied.store(false, std::memory_order_relaxed); 
    
    return true;
}
//----------------------------------------------------------------------------------
template<typename T> size_t
Queue<T>::getSize()
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
// template class Queue<Trade>;