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
                                                     int max_dim = 2,
                                                     double threshold = 2.5);

PersistenceResult compute_persistence(const Eigen::MatrixXd& point_cloud);

}  // namespace defect_gnn::topology