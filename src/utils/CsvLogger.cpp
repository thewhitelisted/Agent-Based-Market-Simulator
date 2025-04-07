#include "utils/CsvLogger.hpp"
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

CsvLogger::CsvLogger(const std::string& filename) {
    fs::create_directories("logs"); // Create logs/ folder if not present
    out.open(filename);
    out << "timestamp,agent_id,cash,inventory,realized_pnl,unrealized_pnl,total_pnl\n";
}

void CsvLogger::log(int timestamp, const std::vector<std::shared_ptr<Agent>>& agents, double marketPrice) {
    for (const auto& agent : agents) {
        double realized = agent->getRealizedPnL();
        double unrealized = agent->getUnrealizedPnL(marketPrice);
        double total = realized + unrealized;
        out << timestamp << "," << agent->getId() << "," << agent->getCash() << "," << agent->getInventory()
            << "," << realized << "," << unrealized << "," << total << "\n";
    }
}

void CsvLogger::close() {
    if (out.is_open()) {
        out.close();
    }
}
