#pragma once

#include <vector>
#include <memory>
#include "core/OrderBook.hpp"
#include "agents/Agent.hpp"

class MarketSimulator {
public:
    MarketSimulator();

    void addAgent(std::shared_ptr<Agent> agent);
    void run(int numSteps);

private:
    OrderBook orderBook;
    std::vector<std::shared_ptr<Agent>> agents;
    long timestamp;

    void stepSimulation();  
    void logState() const;
};
