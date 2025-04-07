#include "core/OrderBook.hpp"
#include <iostream>

OrderBook::OrderBook() {}

void OrderBook::addLimitOrder(const Order& order) {
    idLookup[order.id] = order;

    auto& book = (order.side == OrderSide::BUY) ? bids : asks;
    book[order.price].push_back(order);
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
                queue.erase(qIt);
                idLookup.erase(orderId);
                if (queue.empty()) book.erase(priceIt);
                return true;
            }
        }
    }

    return false;
}

std::vector<Fill> OrderBook::matchMarketOrder(const Order& marketOrder) {
    std::vector<Fill> fills;
    Order remaining = marketOrder;

    auto& book = (marketOrder.side == OrderSide::BUY) ? asks : bids;
    auto it = book.begin();

    while (remaining.quantity > 0 && it != book.end()) {
        auto& queue = it->second;
        while (!queue.empty() && remaining.quantity > 0) {
            Order& top = queue.front();

            int fillQty = std::min(remaining.quantity, top.quantity);
            remaining.quantity -= fillQty;
            top.quantity -= fillQty;

            Fill passiveFill{
                .agentId = top.agentId,
                .price = top.price,
                .quantity = fillQty,
                .side = top.side,  // passive side
                .timestamp = marketOrder.timestamp
            };
            recentFills.push_back(passiveFill);

            Fill activeFill{
                .agentId = marketOrder.agentId,
                .price = top.price,
                .quantity = fillQty,
                .side = (marketOrder.side == OrderSide::BUY ? OrderSide::BUY : OrderSide::SELL),
                .timestamp = marketOrder.timestamp
            };
            fills.push_back(activeFill);          // returned to caller
            recentFills.push_back(activeFill);    // also tracked centrally

            if (top.quantity == 0) {
                idLookup.erase(top.id);
                queue.pop_front();
            }
        }

        if (queue.empty()) {
            it = book.erase(it);
        } else {
            ++it;
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
    std::cout << "Asks:\n";
    for (const auto& [price, queue] : asks) {
        std::cout << "  " << price << " x " << queue.size() << "\n";
    }
    std::cout << "Bids:\n";
    for (auto it = bids.rbegin(); it != bids.rend(); ++it) {
        std::cout << "  " << it->first << " x " << it->second.size() << "\n";
    }
}

const std::vector<Fill>& OrderBook::getRecentFills() const {
    return recentFills;
}

void OrderBook::clearFills() {
    recentFills.clear();
}
