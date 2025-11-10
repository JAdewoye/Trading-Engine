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
struct Trade {
    uint64_t timestamp;
    std::string symbol;
    std::string side;
    double price;
};

struct Cell {
    std::atomic_bool occupied;
    Trade trade;

Cell& operator=(const Cell& other) {
    occupied.store(other.occupied.load());
    trade = other.trade;
    return *this;
}
};
//----------------------------------------------------------------------------------
// Class Definitions
//----------------------------------------------------------------------------------
class TradeQueue{
public:
    TradeQueue(const std::size_t capacity);
    ~TradeQueue() = default;
    bool pushBack(uint64_t timestamp, const std::string& symbol, const std::string& side, double price);
    bool popFront(Cell& result);
    size_t getSize();

private:
    std::vector<Cell> buffer_;
    alignas(64) std::atomic_size_t front_;
    alignas(64) std::atomic_size_t back_;
    const size_t capacity_;
};