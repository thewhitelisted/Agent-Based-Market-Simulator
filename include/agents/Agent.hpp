#pragma once

#include <deque>
#include "core/Order.hpp"

class Agent {
public:
    Agent(int id);
    double startCash = 10000.0;
    virtual ~Agent() = default;

    virtual void act(class OrderBook& book, long timestamp) = 0;
    virtual void onFill(const Fill& fill);

    // Accessor methods
    int getId() const;
    double getCash() const;
    int getInventory() const;
    double getRealizedPnL() const;
    double getUnrealizedPnL(double marketPrice) const;
    
    // Resource management methods
    int getAvailableInventory() const;
    double getAvailableCash() const;
    void cancelReservation(OrderSide side, int quantity, double price);

protected:
    int id;
    double cash;
    int inventory;
    double realizedPnL;
    
    int reservedLongInventory;
    int reservedShortInventory;
    double reservedCash;
    
    std::deque<std::pair<int, double>> positionQueue;
};
