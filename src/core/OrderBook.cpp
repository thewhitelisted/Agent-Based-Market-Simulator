#include "core/OrderBook.hpp"
#include <iostream>
#include <iomanip>  // for setprecision

OrderBook::OrderBook() 
    : lastTradePrice(100.0),  // Initialize with a reasonable default
      actionTakenByAgentId(-1) {}

void OrderBook::addLimitOrder(const Order& order) {
    // First check if order can be immediately matched
    if (order.side == OrderSide::BUY && !asks.empty() && order.price >= asks.begin()->first) {
        auto fills = matchMarketOrder(order);
        if (fills.size() > 0) return; // Order was fully filled
    }
    else if (order.side == OrderSide::SELL && !bids.empty() && order.price <= bids.rbegin()->first) {
        auto fills = matchMarketOrder(order);
        if (fills.size() > 0) return; // Order was fully filled
    }

    // If we get here, either no matching or partial fill - add remaining to book
    static int nextOrderId = 1;
    Order orderWithId = order;
    orderWithId.id = nextOrderId++;
    
    // Add order to the book
    idLookup[orderWithId.id] = orderWithId;
    auto& book = (orderWithId.side == OrderSide::BUY) ? bids : asks;
    book[orderWithId.price].push_back(orderWithId);
    
    // Mark the agent as having taken action
    actionTakenByAgentId = orderWithId.agentId;
    
    // Create a reservation fill for tracking purposes
    Fill reservation{
        .agentId = orderWithId.agentId,
        .price = orderWithId.price,
        .quantity = orderWithId.quantity,
        .side = orderWithId.side,
        .timestamp = orderWithId.timestamp,
        .isReservation = true
    };
    
    recentFills.push_back(reservation);
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
                Fill cancelFill{
                    .agentId = order.agentId,
                    .price = order.price,
                    .quantity = order.quantity,
                    .side = order.side,
                    .timestamp = order.timestamp,
                    .isReservation = true,
                    .isCancellation = true
                };
                recentFills.push_back(cancelFill);
                
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

        while (!orderQueue.empty() && remainingQty > 0) {
            Order& passiveOrder = orderQueue.front();
            
            // Skip self-trades but preserve the order for other agents
            if (passiveOrder.agentId == marketOrder.agentId) {
                continue; 
            }

            int fillQty = std::min(remainingQty, passiveOrder.quantity);

            // Update last trade price
            lastTradePrice = passiveOrder.price;

            // Passive order fill (agent who placed the limit order)
            Fill passiveFill{
                .agentId = passiveOrder.agentId,
                .price = passiveOrder.price,
                .quantity = fillQty,
                .side = passiveOrder.side,
                .timestamp = marketOrder.timestamp,
                .isReservation = false
            };
            recentFills.push_back(passiveFill);

            // Active order fill (agent who placed the market order)
            Fill activeFill{
                .agentId = marketOrder.agentId,
                .price = passiveOrder.price,
                .quantity = fillQty,
                .side = marketOrder.side,
                .timestamp = marketOrder.timestamp,
                .isReservation = false
            };
            fills.push_back(activeFill);

            // Update quantities
            remainingQty -= fillQty;
            passiveOrder.quantity -= fillQty;

            // Remove filled passive orders
            if (passiveOrder.quantity == 0) {
                idLookup.erase(passiveOrder.id);
                orderQueue.pop_front();
            }
        }

        // Remove empty price levels
        if (orderQueue.empty()) {
            book.erase(priceIt);
        } else {
            // No more liquidity at next best price to continue filling
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

