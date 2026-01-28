#pragma once

#include <Eigen/Dense>

#include <string>
#include <vector>

namespace defect_gnn::io {

struct DefectEntry {
    int pris_idx;
    int vac_idx;
    double energy;
    std::string vac_type;
    double formation_energy;
};

std::vector<DefectEntry> parse_defect_csv(const std::string& filepath);

}  // namespace defect_gnn::io