#include "graph/edge_features.hpp"

#include <Eigen/Dense>

namespace defect_gnn::graph {

Eigen::VectorXd gaussian_rbf(double distance,  // NOLINT(bugprone-easily-swappable-parameters)
                             double r_cutoff,
                             double dr) {
    int n = std::floor(r_cutoff / dr);
    double sigma = r_cutoff / 3;
    double inv_sigma_squared = 1 / std::pow(sigma, 2);
    double norm = 1 / (sigma * std::sqrt(2 * M_PI));

    Eigen::VectorXd g;
    g.resize(n);

    for (int k = 0; k < n; k++) {
        double center = k * dr;
        g[k] = (norm * std::exp(-0.5 * std::pow(center - distance, 2) * inv_sigma_squared));
    }

    return g;
}

}  // namespace defect_gnn::graph