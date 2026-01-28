#pragma once

#include <Eigen/Dense>

#include <string>
#include <vector>

namespace defect_gnn::io {

struct VASPStructure {
    Eigen::Matrix3d lattice;
    std::vector<std::string> elements;
    std::vector<int> counts;
    Eigen::MatrixXd frac_coords;
    std::vector<int> atom_types;
};

VASPStructure parse_vasp(const std::string& filepath);

Eigen::MatrixXd frac_to_cart(const Eigen::Matrix3d& lattice, const Eigen::MatrixXd& fract_coords);

}  // namespace defect_gnn::io