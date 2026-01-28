#include "io/csv_parser.hpp"

#include <Eigen/Dense>

#include <fstream>
#include <string>
#include <vector>

namespace defect_gnn::io {

std::vector<DefectEntry> parse_defect_csv(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    std::vector<DefectEntry> entries;

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        DefectEntry entry;
        std::string token;

        std::getline(ss, token, ',');
        entry.pris_idx = std::stoi(token);

        std::getline(ss, token, ',');
        entry.vac_idx = std::stoi(token);

        std::getline(ss, token, ',');
        entry.energy = std::stod(token);

        std::getline(ss, token, ',');
        entry.vac_type = token;

        std::getline(ss, token, ',');
        entry.formation_energy = std::stod(token);

        entries.push_back(entry);
    }

    return entries;
}

}  // namespace defect_gnn::io