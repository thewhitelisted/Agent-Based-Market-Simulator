#pragma once

#include "core/Order.hpp"
#include <deque>

class Agent {
public:
    Agent(int id);
    virtual ~Agent() = default;

    virtual void act(class OrderBook& book, long timestamp) = 0;
    virtual void onFill(const Fill& fill);

    int getId() const;

    double getCash() const;
    int getInventory() const;
    double getRealizedPnL() const;

protected:
    int id;
    int inventory = 0;
    double cash = 0.0;
    double realizedPnL = 0.0;

    std::deque<std::pair<int, double>> positionQueue; // (qty, price)
};
