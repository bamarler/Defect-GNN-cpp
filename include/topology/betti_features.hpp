#pragma once

#include "crystal/structure.hpp"
#include "topology/ripser_wrapper.hpp"

#include <Eigen/Dense>

#include <vector>
namespace defect_gnn::topology {

static constexpr int BETTI_FEATURE_DIM = 35;

struct BettiStatistics {
    double mean = 0.0;
    double std = 0.0;
    double max = 0.0;
    double min = 0.0;
    double weighted_sum = 0.0;
};

inline void append_to(BettiStatistics stats, std::vector<double>& vec) {
    vec.insert(vec.end(), {stats.mean, stats.std, stats.max, stats.min, stats.weighted_sum});
}

BettiStatistics compute_statistics(const PersistenceDiagram& diagram,
                                   const std::string& values_type,
                                   double weight = 1.0);

Eigen::VectorXd
compute_atom_betti_features(const crystal::Structure& structure, size_t atom_idx, double r_cutoff);

Eigen::MatrixXd compute_structure_betti_features(const crystal::Structure& structure,
                                                 double r_cutoff = 2.5);

}  // namespace defect_gnn::topology