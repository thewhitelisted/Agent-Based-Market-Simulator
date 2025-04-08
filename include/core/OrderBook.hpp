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
    double getLastTradePrice() const;
    
    // Action tracking methods
    bool wasActionTakenByAgent(int agentId) const;
    void clearAgentActionFlag();
    
    // Access methods for order books
    const std::map<double, std::deque<Order>>& getAsks() const { return asks; }
    const std::map<double, std::deque<Order>>& getBids() const { return bids; }

private:
    std::map<double, std::deque<Order>> bids; // price -> orders (BUY)
    std::map<double, std::deque<Order>> asks; // price -> orders (SELL)
    std::map<int, Order> idLookup;
    std::vector<Fill> recentFills;
    double lastTradePrice;
    int actionTakenByAgentId;
};
