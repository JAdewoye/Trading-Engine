#pragma once
//----------------------------------------------------------------------------------
// Inlcudes
//----------------------------------------------------------------------------------
#include <string>
#include <atomic>
#include <vector>
//----------------------------------------------------------------------------------
// Struct Definitions
//----------------------------------------------------------------------------------
template<typename T>
struct Cell {
    std::atomic_bool occupied;
    T entry;

Cell& operator=(const Cell& other) {
    occupied.store(other.occupied.load());
    entry = other.entry;
    return *this;
}
};
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
template<typename T>
class Queue{
public:
    Queue(const std::size_t capacity);
    ~Queue() = default;
    bool pushBack(const Cell<T>& entry);
    bool popFront(Cell<T>& result);
    size_t getSize();

private:
    std::vector<Cell<T>> buffer_;
    alignas(64) std::atomic_size_t front_;
    alignas(64) std::atomic_size_t back_;
    const size_t capacity_;
};