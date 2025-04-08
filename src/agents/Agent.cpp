#include "agents/Agent.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>

Agent::Agent(int id) 
    : id(id), 
      cash(10000.0),
      inventory(0),
      realizedPnL(0.0),
      reservedLongInventory(0),
      reservedShortInventory(0),
      reservedCash(0.0) {}

int Agent::getId() const { return id; }
double Agent::getRealizedPnL() const { return realizedPnL; }
double Agent::getCash() const { return cash; }
int Agent::getInventory() const { return inventory; }
int Agent::getAvailableInventory() const { return inventory - reservedShortInventory; }
double Agent::getAvailableCash() const { return cash - reservedCash; }

void Agent::cancelReservation(OrderSide side, int quantity, double price) {
    if (side == OrderSide::BUY) {
        reservedCash -= quantity * price;
        reservedLongInventory -= quantity;
    } else {
        reservedShortInventory -= quantity;
    }
}

void Agent::onFill(const Fill& fill) {
    // Handle reservation fills
    if (fill.isReservation) {
        if (fill.side == OrderSide::BUY) {
            reservedCash += fill.quantity * fill.price;
            reservedLongInventory += fill.quantity;
        } else {
            reservedShortInventory += fill.quantity;
        }
        return;
    }

    int qty = fill.quantity;
    double price = fill.price;
    
    // Determine if we're buying or selling
    bool isBuying = (fill.side == OrderSide::SELL);
    
    // Log the fill
    std::cout << "Agent " << id << " " << (isBuying ? "BUY" : "SELL") 
              << " " << qty << " @ " << std::fixed << std::setprecision(2) << price << std::endl;
    
    // Update inventory and cash
    if (isBuying) {
        // We're buying
        cash -= qty * price;
        inventory += qty;
        
        // Release reserved cash for limit orders
        if (fill.agentId == id) {
            reservedCash = std::max(0.0, reservedCash - (qty * price));
            reservedLongInventory = std::max(0, reservedLongInventory - qty);
        }
        
        // Handle covering short positions
        int remainingQty = qty;
        while (remainingQty > 0 && !positionQueue.empty() && positionQueue.front().first < 0) {
            auto& [posQty, posPrice] = positionQueue.front();
            int coverQty = std::min(remainingQty, -posQty);
            
            double pnl = (posPrice - price) * coverQty;
            realizedPnL += pnl;
            
            std::cout << "  Covered " << coverQty << " shorts @ " << posPrice 
                      << ", bought @ " << price << ", PnL: " << pnl << std::endl;
            
            posQty += coverQty;
            remainingQty -= coverQty;
            
            if (posQty == 0) {
                positionQueue.pop_front();
            }
        }
        
        // Add remaining as new long position
        if (remainingQty > 0) {
            positionQueue.push_back({remainingQty, price});
            std::cout << "  New long position: " << remainingQty << " @ " << price << std::endl;
        }
    } else {
        // We're selling
        cash += qty * price;
        inventory -= qty;
        
        // Release reserved inventory for limit orders
        if (fill.agentId == id) {
            reservedShortInventory = std::max(0, reservedShortInventory - qty);
        }
        
        // Handle selling long positions
        int remainingQty = qty;
        while (remainingQty > 0 && !positionQueue.empty() && positionQueue.front().first > 0) {
            auto& [posQty, posPrice] = positionQueue.front();
            int sellQty = std::min(remainingQty, posQty);
            
            double pnl = (price - posPrice) * sellQty;
            realizedPnL += pnl;
            
            std::cout << "  Sold " << sellQty << " longs @ " << posPrice 
                      << ", sold @ " << price << ", PnL: " << pnl << std::endl;
            
            posQty -= sellQty;
            remainingQty -= sellQty;
            
            if (posQty == 0) {
                positionQueue.pop_front();
            }
        }
        
        // Add remaining as new short position
        if (remainingQty > 0) {
            positionQueue.push_back({-remainingQty, price});
            std::cout << "  New short position: " << -remainingQty << " @ " << price << std::endl;
        }
    }
}

double Agent::getUnrealizedPnL(double marketPrice) const {
    if (inventory == 0) return 0.0;
    
    double unrealizedPnL = 0.0;
    for (const auto& [qty, entryPrice] : positionQueue) {
        if (qty > 0) {
            unrealizedPnL += (marketPrice - entryPrice) * qty;
        } else if (qty < 0) {
            unrealizedPnL += (entryPrice - marketPrice) * (-qty);
        }
    }
    return unrealizedPnL;
}
