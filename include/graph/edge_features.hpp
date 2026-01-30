#pragma once

#include <Eigen/Dense>

namespace defect_gnn::graph {

// Gaussian radial basis function expansion
// r_cutoff=10.0, dr=0.1 → 100 bins
// sigma = r_cutoff / 3
//
// Formula: exp(-0.5 * (r_grid - dist)^2 / sigma^2) / sqrt(2*pi) / sigma
//
// Reference: utilities.py:25-32
Eigen::VectorXd gaussian_rbf(double distance, double r_cutoff, double dr);

}  // namespace defect_gnn::graph