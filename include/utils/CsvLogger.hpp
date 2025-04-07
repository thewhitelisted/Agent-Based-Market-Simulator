#pragma once

#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include "agents/Agent.hpp"

class CsvLogger {
public:
    CsvLogger(const std::string& filename);
    void log(int timestamp, const std::vector<std::shared_ptr<Agent>>& agents, double marketPrice);
    void close();

private:
    std::ofstream out;
};
