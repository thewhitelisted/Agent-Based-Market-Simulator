#include <iostream>
#include <memory>
#include "core/MarketSimulator.hpp"
#include "agents/NoiseTrader.hpp"

int main() {
    std::cout << "Adversarial Market Simulation Starting...\n";

    MarketSimulator sim(10); // 10 steps simulation
    
    // Add agents with initial cash of 10,000 each (as defined in Agent constructor)
    sim.addAgent(std::make_shared<NoiseTrader>(301));
    sim.addAgent(std::make_shared<NoiseTrader>(302));

    sim.run();

    return 0;
}
