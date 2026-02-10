#pragma once

#include <Eigen/Dense>

#include <vector>

namespace defect_gnn::topology {

struct PersistencePair {
    double birth;
    double death;
};

[[nodiscard]] inline double persistence(const PersistencePair& p) {
    return p.death - p.birth;
}

using PersistenceDiagram = std::vector<PersistencePair>;

struct PersistenceResult {
    PersistenceDiagram dim0;
    PersistenceDiagram dim1;
    PersistenceDiagram dim2;
};

PersistenceResult compute_persistence_from_distances(const Eigen::MatrixXd& distance_matrix,
                                                     double threshold,
                                                     unsigned num_threads);

PersistenceResult
compute_persistence(const Eigen::MatrixXd& point_cloud, double threshold, unsigned num_threads);

}  // namespace defect_gnn::topology