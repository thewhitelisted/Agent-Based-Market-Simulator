#pragma once

#include <map>
#include <deque>
#include <vector>
#include <optional>
#include "Order.hpp"

class OrderBook {
public:
    OrderBook();

    void addLimitOrder(const Order& order);
    std::vector<Fill> matchMarketOrder(const Order& marketOrder);
    bool cancelOrder(int orderId);

    std::optional<double> bestBid() const;
    std::optional<double> bestAsk() const;

    void printBook() const;

    const std::vector<Fill>& getRecentFills() const;
    void clearFills();
    double getMidPrice() const;

private:
    std::map<double, std::deque<Order>> bids; // price -> orders (BUY)
    std::map<double, std::deque<Order>> asks; // price -> orders (SELL)
    std::map<int, Order> idLookup;
    std::vector<Fill> recentFills; 
};
