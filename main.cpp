#include <iostream>
#include <memory>
#include "core/MarketSimulator.hpp"
#include "agents/NoiseTrader.hpp"

int main() {
    std::cout << "Adversarial Market Simulation Starting...\n";

    MarketSimulator sim;

    // Add any number of agents
    sim.addAgent(std::make_shared<NoiseTrader>(301));
    sim.addAgent(std::make_shared<NoiseTrader>(302));

    sim.run(10); // 10 time steps

    return 0;
}
