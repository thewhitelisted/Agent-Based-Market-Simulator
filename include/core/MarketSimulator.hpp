#pragma once

#include "core/OrderBook.hpp"
#include "utils/CsvLogger.hpp"
#include "agents/Agent.hpp"
#include <memory>
#include <vector>

class MarketSimulator {
public:
    MarketSimulator(int steps);
    void addAgent(std::shared_ptr<Agent> agent);
    void run();
    void stepSimulation();

private:
    int timestamp;
    int maxSteps;
    OrderBook orderBook;
    std::vector<std::shared_ptr<Agent>> agents;
    std::unique_ptr<CsvLogger> logger;
};
