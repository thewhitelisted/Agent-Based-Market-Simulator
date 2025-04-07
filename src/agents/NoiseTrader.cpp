#include "agents/NoiseTrader.hpp"
#include <iostream>

NoiseTrader::NoiseTrader(int id)
    : Agent(id), rng(std::random_device{}()),
      priceDist(99.0, 101.0),
      sideDist(0, 1),
      qtyDist(1, 10),
      typeDist(0, 1) {}

void NoiseTrader::act(OrderBook& book, long timestamp) {
    OrderSide side = (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL;
    int qty = qtyDist(rng);

    if (typeDist(rng) == 0) {
        // Place a limit order
        double price = priceDist(rng);
        Order order{-1, id, price, qty, side, timestamp};
        book.addLimitOrder(order);
        std::cout << "[NoiseTrader " << id << "] Placed LIMIT " << (side == OrderSide::BUY ? "BUY" : "SELL")
                  << " " << qty << " @ " << price << "\n";
    } else {
        Order order{-1, id, 0.0, qty, side, timestamp};
        auto fills = book.matchMarketOrder(order);
        std::cout << "[NoiseTrader " << id << "] Placed MARKET " 
                << (side == OrderSide::BUY ? "BUY" : "SELL")
                << " " << qty << "\n";

        for (const auto& fill : fills) {
            onFill(fill); 
            std::cout << "  -> Filled " << fill.quantity << " @ " << fill.price << "\n";
        }
    }
}
