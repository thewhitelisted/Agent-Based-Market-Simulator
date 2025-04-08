# 🧠 Agent-Based Market Simulator

A modular, agent-based simulation of a limit order book exchange in C++. Designed for researching adversarial trading behavior, market making, and inventory-sensitive strategies in a competitive environment.

---

## 🚀 Features

- Central Limit Order Book (CLOB) engine
- Support for market and limit orders
- Active/passive trade handling and fill routing
- Inventory, cash, and realized PnL tracking (with FIFO cost basis)
- Fully autonomous agent framework
- NoiseTrader agents with randomized behavior
- Configurable simulation steps
- Clean CMake-powered project structure

---

## 🏗️ Project Structure

```
adversarial_sim/
├── CMakeLists.txt         # Project build file
├── include/
│   ├── core/              # Market core components
│   │   ├── OrderBook.hpp
│   │   └── MarketSimulator.hpp
│   └── agents/            # Agent base + strategies
│       ├── Agent.hpp
│       └── NoiseTrader.hpp
├── src/
│   ├── core/              # OrderBook + MarketSimulator implementations
│   └── agents/            # Agent logic
├── main.cpp               # Entry point
├── build/                 # (Generated) Build output
└── README.md              # This file
```

---

## ⚙️ Build Instructions (Windows, CMake)

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
.\Release dversarial_sim.exe
```

✅ Requires:
- CMake (v3.15+)
- MSVC or compatible C++20 compiler

---

## 📈 Simulation Output

Each run will:
- Print timestamped simulation steps
- Show agent actions and order book state
- Track per-agent:
  - Inventory
  - Cash balance
  - Realized PnL (via FIFO queue)

---

## 🧪 Agent Types

| Agent        | Description |
|--------------|-------------|
| `NoiseTrader`| Places random market/limit orders |
| *(Planned)* `MarketMaker` | Two-sided quoting, inventory risk-managed |
| *(Planned)* `Spoofer`     | Strategic misleading orders |
| *(Planned)* `Sniper`      | Latency arb / reaction-based |

---

## 🔮 Future Roadmap

- [x] Unrealized PnL (mark-to-market)
- [x] CSV output / data logging
- [ ] Visualization support
- [ ] Order latency modeling
- [ ] Adversarial & strategic agents