#pragma once

#include "core/OrderBook.hpp"
#include "utils/CsvLogger.hpp"
#include "agents/Agent.hpp"
#include <memory>
#include <vector>

class MarketSimulator {
public:
    // Constructor requires the total simulation steps.
    MarketSimulator(int steps);
    
    // Add an agent to the simulation.
    void addAgent(std::shared_ptr<Agent> agent);
    
    // Run the full simulation.
    void run();
    
    // Execute one simulation step.
    void stepSimulation();
    
private:
    int timestamp;
    int maxSteps;
    OrderBook orderBook;
    std::vector<std::shared_ptr<Agent>> agents;
    std::unique_ptr<CsvLogger> logger;
};
