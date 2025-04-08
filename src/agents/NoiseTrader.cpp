#include "agents/NoiseTrader.hpp"
#include <iostream>
#include <iomanip>
#include <stdexcept>

NoiseTrader::NoiseTrader(int id)
    : Agent(id), rng(std::random_device{}()),
      priceDist(99.0, 101.0),
      sideDist(0, 1),
      qtyDist(1, 10),
      typeDist(0, 1) {}

void NoiseTrader::act(OrderBook& book, long timestamp) {
    try {
        // Try to place a limit order first
        if (typeDist(rng) == 0) {
            if (tryLimitOrder(book, timestamp)) {
                return;
            }
        }
        
        // If limit order doesn't work or we chose market order, try that
        if (tryMarketOrder(book, timestamp)) {
            return;
        }
        
        // If both strategies fail, do nothing this turn
        std::cout << "[NoiseTrader " << id << "] No valid action available\n";
        
    } catch (const std::exception& e) {
        std::cerr << "[NoiseTrader " << id << "] Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[NoiseTrader " << id << "] Unknown error" << std::endl;
    }
}

bool NoiseTrader::tryLimitOrder(OrderBook& book, long timestamp) {
    OrderSide side = (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL;
    int qty = std::max(1, std::min(qtyDist(rng), 10));
    
    // Get price - slightly offset from midpoint for better execution chance
    double midPrice = book.getMidPrice();
    double price = (side == OrderSide::BUY) ? 
        midPrice * 0.995 + priceDist(rng) * 0.01 : 
        midPrice * 1.005 - priceDist(rng) * 0.01;
    
    // Ensure price is reasonable
    price = std::max(95.0, std::min(price, 105.0));
    
    // Check resource constraints
    if (side == OrderSide::BUY) {
        double cost = qty * price;
        if (getAvailableCash() < cost) {
            qty = std::max(1, static_cast<int>(getAvailableCash() / price));
            if (qty < 1) return false;
        }
    } else {
        // Allow short selling up to same limit as longs
        int maxShort = static_cast<int>(getAvailableCash() / price);
        qty = std::min(qty, maxShort);  // Limit short position size
        if (qty < 1) return false;
    }
    
    // Place order
    Order order{-1, id, price, qty, side, timestamp};
    book.addLimitOrder(order);
    std::cout << "[NoiseTrader " << id << "] Placed LIMIT " 
              << (side == OrderSide::BUY ? "BUY" : "SELL")
              << " " << qty << " @ " << std::fixed << std::setprecision(2) << price << "\n";
    return true;
}

bool NoiseTrader::tryMarketOrder(OrderBook& book, long timestamp) {
    // Try the side with available liquidity first
    bool buyLiquidity = book.bestAsk().has_value();
    bool sellLiquidity = book.bestBid().has_value();
    
    OrderSide side;
    if (buyLiquidity && sellLiquidity) {
        // Both sides available - use random
        side = (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL;
    } else if (buyLiquidity) {
        side = OrderSide::BUY;
    } else if (sellLiquidity) {
        side = OrderSide::SELL;
    } else {
        return false;  // No liquidity on either side
    }
    
    // Adjust quantity based on available resources
    int maxQty = std::min(qtyDist(rng), 10);
    int qty;
    if (side == OrderSide::BUY) {
        double availableCash = getAvailableCash();
        double estimatedPrice = book.bestAsk().value_or(100.0);
        qty = std::max(1, std::min(maxQty, static_cast<int>(availableCash / estimatedPrice)));
    } else {
        // Allow short selling with similar size constraints as buys
        double estimatedPrice = book.bestBid().value_or(100.0);
        int maxShort = static_cast<int>(getAvailableCash() / estimatedPrice);
        qty = std::min(maxQty, maxShort);
    }
    
    if (qty < 1) return false;
    
    Order order{-1, id, 0.0, qty, side, timestamp};
    auto fills = book.matchMarketOrder(order);
    
    std::cout << "[NoiseTrader " << id << "] Placed MARKET " 
              << (side == OrderSide::BUY ? "BUY" : "SELL")
              << " " << qty << "\n";
              
    if (!fills.empty()) {
        for (const auto& fill : fills) {
            std::cout << "  -> Filled " << fill.quantity 
                      << " @ " << std::fixed << std::setprecision(2) << fill.price << "\n";
        }
        return true;
    }
    
    std::cout << "  -> No fills: insufficient liquidity\n";
    return false;
}