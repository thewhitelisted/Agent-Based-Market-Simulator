#include "agents/Agent.hpp"
#include <algorithm>

Agent::Agent(int id) : id(id) {}

int Agent::getId() const { return id; }

double Agent::getCash() const { return cash; }
int Agent::getInventory() const { return inventory; }
double Agent::getRealizedPnL() const { return realizedPnL; }

void Agent::onFill(const Fill& fill) {
    int qty = fill.quantity;
    double price = fill.price;

    bool isBuy = (fill.side == OrderSide::SELL); // If counterparty sold, we bought
    int signedQty = isBuy ? qty : -qty;

    inventory += signedQty;
    cash += -signedQty * fill.price;


    if (positionQueue.empty() || 
        (isBuy && inventory > 0) || 
        (!isBuy && inventory < 0)) {
        // Opening or increasing same-side position
        positionQueue.emplace_back(qty, price);
    } else {
        // Reducing opposite-side position
        while (qty > 0 && !positionQueue.empty()) {
            auto& [openQty, openPrice] = positionQueue.front();
            int closeQty = std::min(qty, openQty);
            double pnlPerUnit = isBuy ? (openPrice - price) : (price - openPrice);
            realizedPnL += closeQty * pnlPerUnit;

            openQty -= closeQty;
            qty -= closeQty;

            if (openQty == 0) positionQueue.pop_front();
        }

        // If remaining qty still extends current position direction
        if (qty > 0) {
            positionQueue.emplace_back(qty, price);
        }
    }
}

double Agent::getVWAP() const {
    if (inventory == 0 || positionQueue.empty()) return 0.0;

    double totalCost = 0.0;
    int totalQty = 0;

    for (const auto& [qty, price] : positionQueue) {
        totalCost += qty * price;
        totalQty += qty;
    }

    return totalQty > 0 ? totalCost / totalQty : 0.0;
}

double Agent::getUnrealizedPnL(double marketPrice) const {
    if (inventory == 0) return 0.0;

    double avgEntry = getVWAP();
    return (marketPrice - avgEntry) * inventory;
}
