#pragma once

#include "agents/Agent.hpp"
#include <random>
#include "core/OrderBook.hpp"

class NoiseTrader : public Agent {
public:
    NoiseTrader(int id);

    void act(OrderBook& book, long timestamp) override;

private:
    // Order placement strategies
    bool tryLimitOrder(OrderBook& book, long timestamp);
    bool tryMarketOrder(OrderBook& book, long timestamp);
    void placeGuaranteedOrder(OrderBook& book, long timestamp);

    std::mt19937 rng;
    std::uniform_real_distribution<> priceDist;
    std::uniform_int_distribution<> sideDist;
    std::uniform_int_distribution<> qtyDist;
    std::uniform_int_distribution<> typeDist;
};
