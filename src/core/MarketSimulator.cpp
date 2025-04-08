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

    // Get the market price for PnL calculations: prefer last trade price, fallback to mid price.
    double marketPrice = orderBook.getLastTradePrice();
    if (marketPrice == 0) {
        marketPrice = orderBook.getMidPrice();
    }
    
    std::cout << "\nMarket price used for PnL calculations: " 
              << std::fixed << std::setprecision(2) << marketPrice << std::endl;

    // Log the current state.
    logger->log(timestamp, agents, marketPrice);

    std::cout << "--- Timestamp: " << timestamp << " ---\n";
    orderBook.printBook();

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& agent : agents) {
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
