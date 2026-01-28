#pragma once

#include <Eigen/Dense>

#include <map>
#include <string>

namespace defect_gnn::io {

std::map<int, Eigen::VectorXd> parse_atom_embeddings(const std::string& filepath);

}  // namespace defect_gnn::io