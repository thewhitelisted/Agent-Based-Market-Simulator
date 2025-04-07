#include "core/MarketSimulator.hpp"
#include <iostream>
#include <memory>

MarketSimulator::MarketSimulator(int steps)
    : timestamp(0),
      maxSteps(steps),
      logger(std::make_unique<CsvLogger>("logs/simulation.csv")) {}

void MarketSimulator::addAgent(std::shared_ptr<Agent> agent) {
    agents.push_back(agent);
}

void MarketSimulator::stepSimulation() {
    for (auto& agent : agents) {
        agent->act(orderBook, timestamp);
    }

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

    double marketPrice = orderBook.getMidPrice();
    logger->log(timestamp, agents, marketPrice); 

    std::cout << "--- Timestamp: " << timestamp << " ---\n";
    orderBook.printBook();

    for (const auto& agent : agents) {
        double realized = agent->getRealizedPnL();
        double unrealized = agent->getUnrealizedPnL(marketPrice);
        std::cout << "Agent " << agent->getId()
                  << " | Cash: " << agent->getCash()
                  << " | Inventory: " << agent->getInventory()
                  << " | Realized PnL: " << realized
                  << " | Unrealized PnL: " << unrealized
                  << " | Total PnL: " << (realized + unrealized)
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
