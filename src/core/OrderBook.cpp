#include "core/OrderBook.hpp"
#include <iostream>
#include <iomanip>  // for setprecision

OrderBook::OrderBook() 
    : lastTradePrice(100.0),  // Initialize with a reasonable default
      actionTakenByAgentId(-1) {}

void OrderBook::addLimitOrder(const Order& order) {
    int remainingQty = order.quantity;
    
    // First check if order can be immediately matched
    if (order.side == OrderSide::BUY && !asks.empty() && order.price >= asks.begin()->first) {
        auto fills = matchMarketOrder(order);
        if (!fills.empty()) {
            // Track remaining quantity
            for (const auto& fill : fills) {
                remainingQty -= fill.quantity;
            }
            // If order was fully filled, we're done
            if (remainingQty == 0) {
                return;
            }
        }
    }
    else if (order.side == OrderSide::SELL && !bids.empty() && order.price <= bids.rbegin()->first) {
        auto fills = matchMarketOrder(order);
        if (!fills.empty()) {
            // Track remaining quantity
            for (const auto& fill : fills) {
                remainingQty -= fill.quantity;
            }
            // If order was fully filled, we're done
            if (remainingQty == 0) {
                return;
            }
        }
    }

    // If we get here, either no matching or partial fill - add remaining to book
    static int nextOrderId = 1;
    Order orderWithId = order;
    orderWithId.id = nextOrderId++;
    orderWithId.quantity = remainingQty;  // Update with remaining quantity
    
    // Add order to the book
    idLookup[orderWithId.id] = orderWithId;
    auto& book = (orderWithId.side == OrderSide::BUY) ? bids : asks;
    book[orderWithId.price].push_back(orderWithId);
    
    // Mark the agent as having taken action
    actionTakenByAgentId = orderWithId.agentId;
    
    // Create a reservation fill for tracking purposes
    recentFills.emplace_back(Fill{
        .agentId = orderWithId.agentId,
        .price = orderWithId.price,
        .quantity = orderWithId.quantity,
        .side = orderWithId.side,
        .timestamp = orderWithId.timestamp,
        .isReservation = true
    });
}

bool OrderBook::cancelOrder(int orderId) {
    auto it = idLookup.find(orderId);
    if (it == idLookup.end()) return false;

    const Order& order = it->second;
    auto& book = (order.side == OrderSide::BUY) ? bids : asks;

    auto priceIt = book.find(order.price);
    if (priceIt != book.end()) {
        auto& queue = priceIt->second;
        for (auto qIt = queue.begin(); qIt != queue.end(); ++qIt) {
            if (qIt->id == orderId) {
                // Create reservation cancellation fill
                recentFills.emplace_back(Fill{
                    .agentId = order.agentId,
                    .price = order.price,
                    .quantity = order.quantity,
                    .side = order.side,
                    .timestamp = order.timestamp,
                    .isReservation = true,
                    .isCancellation = true
                });
                
                // Remove the order
                queue.erase(qIt);
                idLookup.erase(orderId);
                
                // Clean up empty price levels
                if (queue.empty()) book.erase(priceIt);
                
                // Mark the agent as having taken action
                actionTakenByAgentId = order.agentId;
                
                return true;
            }
        }
    }

    return false;
}

std::vector<Fill> OrderBook::matchMarketOrder(const Order& marketOrder) {
    std::vector<Fill> fills;
    if (marketOrder.quantity <= 0 || marketOrder.agentId < 0) return fills;
    
    actionTakenByAgentId = marketOrder.agentId;
    int remainingQty = marketOrder.quantity;
    auto& book = (marketOrder.side == OrderSide::BUY) ? asks : bids;

    if (book.empty()) return fills;

    while (remainingQty > 0 && !book.empty()) {
        auto priceIt = (marketOrder.side == OrderSide::BUY) ? 
                      book.begin() : std::prev(book.end());
        auto& orderQueue = priceIt->second;
        
        // Use an iterator to track our position in the queue
        auto orderIt = orderQueue.begin();
        
        while (orderIt != orderQueue.end() && remainingQty > 0) {
            Order& passiveOrder = *orderIt;
            
            // Skip self-trades but preserve the order for other agents
            if (passiveOrder.agentId == marketOrder.agentId) {
                // Using iterator to skip without modifying queue structure
                // This preserves FIFO order priority while preventing self-trading
                ++orderIt;
                continue;
            }

            int fillQty = std::min(remainingQty, passiveOrder.quantity);

            // Update last trade price
            lastTradePrice = passiveOrder.price;

            // Passive order fill (agent who placed the limit order)
            recentFills.emplace_back(Fill{
                .agentId = passiveOrder.agentId,
                .price = passiveOrder.price,
                .quantity = fillQty,
                .side = passiveOrder.side,
                .timestamp = marketOrder.timestamp,
                .isReservation = false
            });

            // Active order fill (agent who placed the market order)
            fills.emplace_back(Fill{
                .agentId = marketOrder.agentId,
                .price = passiveOrder.price,
                .quantity = fillQty,
                .side = marketOrder.side,
                .timestamp = marketOrder.timestamp,
                .isReservation = false
            });

            // Update quantities
            remainingQty -= fillQty;
            passiveOrder.quantity -= fillQty;

            // Remove filled passive orders
            if (passiveOrder.quantity == 0) {
                idLookup.erase(passiveOrder.id);
                orderIt = orderQueue.erase(orderIt);
            } else {
                ++orderIt;
            }
        }

        // Remove empty price levels
        if (orderQueue.empty()) {
            book.erase(priceIt);
        } else if (remainingQty > 0) {
            // No more matchable orders at this price level
            break;
        }
    }

    return fills;
}

std::optional<double> OrderBook::bestBid() const {
    if (bids.empty()) return std::nullopt;
    return bids.rbegin()->first;
}

std::optional<double> OrderBook::bestAsk() const {
    if (asks.empty()) return std::nullopt;
    return asks.begin()->first;
}

void OrderBook::printBook() const {
    std::cout << "=== ORDER BOOK ===\n";
    
    // Print asks from highest to lowest
    std::cout << "Asks (Sell Orders):\n";
    if (asks.empty()) {
        std::cout << "  [empty]\n";
    } else {
        for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
            int totalQty = 0;
            for (const auto& order : it->second) {
                totalQty += order.quantity;
            }
            std::cout << "  Price: " << std::fixed << std::setprecision(2) << it->first 
                      << " | Qty: " << totalQty << "\n";
        }
    }
    
    // Print bids from highest to lowest
    std::cout << "Bids (Buy Orders):\n";
    if (bids.empty()) {
        std::cout << "  [empty]\n";
    } else {
        for (auto it = bids.rbegin(); it != bids.rend(); ++it) {
            int totalQty = 0;
            for (const auto& order : it->second) {
                totalQty += order.quantity;
            }
            std::cout << "  Price: " << std::fixed << std::setprecision(2) << it->first 
                      << " | Qty: " << totalQty << "\n";
        }
    }
}

const std::vector<Fill>& OrderBook::getRecentFills() const {
    return recentFills;
}

void OrderBook::clearFills() {
    recentFills.clear();
}

double OrderBook::getMidPrice() const {
    auto bid = bestBid();
    auto ask = bestAsk();
    
    if (bid && ask) {
        // If we have both sides, use the midpoint
        return (bid.value() + ask.value()) / 2.0;
    } else if (bid) {
        // If we only have bids, use the highest bid
        return bid.value();
    } else if (ask) {
        // If we only have asks, use the lowest ask
        return ask.value();
    } else {
        // When no orders exist, use the last trade price
        return lastTradePrice;
    }
}

double OrderBook::getLastTradePrice() const {
    return lastTradePrice;
}

bool OrderBook::wasActionTakenByAgent(int agentId) const {
    return actionTakenByAgentId == agentId;
}

void OrderBook::clearAgentActionFlag() {
    actionTakenByAgentId = -1;
}

