#pragma once

#include "crystal/structure.hpp"
#include "topology/ripser_wrapper.hpp"

#include <Eigen/Dense>
namespace defect_gnn::topology {

struct BettiStatistics {
    double mean, std, max, min, weighted_sum;
};

BettiStatistics compute_statistics(const PersistenceDiagram& diagram, bool use_death = true);

Eigen::VectorXd compute_atom_betti_features(const crystal::Structure& structure,
                                            size_t atom_idx,
                                            double r_cutoff = 2.5);

Eigen::MatrixXd compute_structure_betti_features(const crystal::Structure& structure,
                                                 double r_cutoff = 2.5);

}  // namespace defect_gnn::topology