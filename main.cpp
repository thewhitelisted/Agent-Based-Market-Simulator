#include <iostream>
#include <memory>
#include "core/MarketSimulator.hpp"
#include "agents/NoiseTrader.hpp"

int main() {
    std::cout << "Adversarial Market Simulation Starting...\n";

    MarketSimulator sim(50);
    
    // Add agents with initial cash of 10,000 each (as defined in Agent constructor)
    sim.addAgent(std::make_shared<NoiseTrader>(301));
    sim.addAgent(std::make_shared<NoiseTrader>(302));
    sim.addAgent(std::make_shared<NoiseTrader>(303));
    sim.addAgent(std::make_shared<NoiseTrader>(304));
    sim.addAgent(std::make_shared<NoiseTrader>(305));
    sim.addAgent(std::make_shared<NoiseTrader>(306));
    sim.addAgent(std::make_shared<NoiseTrader>(307));
    sim.addAgent(std::make_shared<NoiseTrader>(308));
    sim.addAgent(std::make_shared<NoiseTrader>(309));
    sim.addAgent(std::make_shared<NoiseTrader>(310));

    sim.run();

    return 0;
}
