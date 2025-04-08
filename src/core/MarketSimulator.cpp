#include "core/MarketSimulator.hpp"
#include <iostream>
#include <iomanip>
#include <memory>

MarketSimulator::MarketSimulator(int steps)
    : timestamp(0),
      maxSteps(steps),
      logger(std::make_unique<CsvLogger>("logs/simulation.csv"))
{
}

void MarketSimulator::addAgent(std::shared_ptr<Agent> agent) {
    agents.push_back(agent);
}

void MarketSimulator::stepSimulation() {
    // Let each agent perform their actions.
    for (auto& agent : agents) {
        agent->act(orderBook, timestamp);
    }

    // Dispatch fills to agents.
    const auto& fills = orderBook.getRecentFills();
    for (const auto& fill : fills) {
        for (auto& agent : agents) {
            if (agent->getId() == fill.agentId) {
                agent->onFill(fill);
                break;
            }
        }
    }
    orderBook.clearFills();

    // Get the market price for PnL calculations
    double lastTradePrice = orderBook.getLastTradePrice();
    auto bestBid = orderBook.bestBid();
    auto bestAsk = orderBook.bestAsk();
    
    std::cout << "\nMarket prices for PnL calculations:\n";
    std::cout << "Last Trade: " << std::fixed << std::setprecision(2) << lastTradePrice << "\n";
    if (bestBid) std::cout << "Best Bid: " << *bestBid << "\n";
    if (bestAsk) std::cout << "Best Ask: " << *bestAsk << "\n";

    // Log the current state.
    logger->log(timestamp, agents, lastTradePrice);

    std::cout << "--- Timestamp: " << timestamp << " ---\n";
    orderBook.printBook();

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& agent : agents) {
        double marketPrice;
        if (agent->getInventory() > 0) {
            // Long position - use best bid (what we could sell at)
            marketPrice = bestBid.value_or(lastTradePrice);
        } else if (agent->getInventory() < 0) {
            // Short position - use best ask (what we could buy at)
            marketPrice = bestAsk.value_or(lastTradePrice);
        } else {
            // Flat position - use last trade price
            marketPrice = lastTradePrice;
        }
        
        // Fallback to default if no prices available
        if (marketPrice == 0) {
            marketPrice = 100.0;
            std::cout << "Warning: Using default price of 100.0 for PnL calculations (no market data available)\n";
        }
        
        double unrealized = agent->getUnrealizedPnL(marketPrice);
        std::cout << "Agent " << agent->getId()
                  << " | Cash: " << agent->getCash()
                  << " (Available: " << agent->getAvailableCash() << ")"
                  << " | Inventory: " << agent->getInventory()
                  << " (Available: " << agent->getAvailableInventory() << ")"
                  << " | Realized PnL: " << agent->getRealizedPnL()
                  << " | Unrealized PnL: " << unrealized
                  << " | Total PnL: " << agent->getRealizedPnL() + unrealized
                  << std::endl;
    }
    
    std::cout << std::endl;
    timestamp++;
}

void MarketSimulator::run() {
    while (timestamp < maxSteps) {
        stepSimulation();
    }
    logger->close();
    std::cout << "=== SIMULATION COMPLETE ===\n";
}
