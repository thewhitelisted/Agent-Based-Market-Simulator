#include "core/MarketSimulator.hpp"
#include <iostream>

MarketSimulator::MarketSimulator() : timestamp(0) {}

void MarketSimulator::addAgent(std::shared_ptr<Agent> agent) {
    agents.push_back(agent);
}

void MarketSimulator::run(int numSteps) {
    for (int step = 0; step < numSteps; ++step) {
        std::cout << "\n--- Timestamp: " << timestamp << " ---\n";
        stepSimulation();
        logState();
        ++timestamp;
    }

    std::cout << "\n=== SIMULATION COMPLETE ===\n";
}

void MarketSimulator::stepSimulation() {
    // Step each agent
    for (auto& agent : agents) {
        agent->act(orderBook, timestamp);
    }

    // Collect fills and dispatch to passive agents
    for (const auto& fill : orderBook.getRecentFills()) {
        for (auto& agent : agents) {
            if (agent->getId() == fill.agentId) {
                agent->onFill(fill); // ✅ Passive fill!
                break;
            }
        }
    }
    orderBook.clearFills(); // reset buffer
}

void MarketSimulator::logState() const {
    orderBook.printBook();

    std::cout << "\n--- Agent Stats ---\n";
    for (const auto& agent : agents) {
        std::cout << "Agent " << agent->getId()
                  << " | Cash: " << agent->getCash()
                  << " | Inventory: " << agent->getInventory()
                  << " | Realized PnL: " << agent->getRealizedPnL()
                  << "\n";
    }
}
