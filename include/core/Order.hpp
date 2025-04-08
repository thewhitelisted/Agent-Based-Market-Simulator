#pragma once

#include <string>

enum class OrderSide { BUY, SELL };

struct Order {
    int id;
    int agentId;
    double price;
    int quantity;
    OrderSide side;
    long timestamp;
};

struct Fill {
    int agentId;
    double price;
    int quantity;
    OrderSide side;
    long timestamp;
    bool isReservation = false;
    bool isCancellation = false;
};
