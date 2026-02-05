#include "topology/ripser_wrapper.hpp"

#define RIPSER_AS_LIBRARY

#include <Eigen/Dense>

#include <ripser/ripser.cpp>  // NOLINT(bugprone-suspicious-include)
#include <vector>

namespace defect_gnn::topology {

PersistenceResult
compute_persistence_from_distances(const Eigen::MatrixXd& distance_matrix,
                                   int max_dim,  // NOLINT(bugprone-easily-swappable-parameters)
                                   double threshold) {
    const auto N = distance_matrix.rows();

    std::vector<value_t> distances;
    distances.reserve(N * (N - 1) / 2);

    for (int i = 1; i < N; i++) {
        for (int j = 1; j < i; j++) {
            distances.push_back(static_cast<value_t>(distance_matrix(i, j)));
        }
    }

    compressed_lower_distance_matrix dist(std::move(distances));

    auto thresh = static_cast<value_t>(threshold);
    coefficient_t modulus = 2;
    float ratio = 1.0;

    ripser<sparse_distance_matrix> r(
        sparse_distance_matrix(dist, thresh), max_dim, thresh, ratio, modulus);
    r.compute_barcodes();

    PersistenceResult result;
    auto convert = [](const std::vector<std::pair<value_t, value_t>>& pairs) {
        PersistenceDiagram diagram;
        diagram.reserve(pairs.size());
        for (const auto& [b, d] : pairs) {
            diagram.push_back({static_cast<double>(b), static_cast<double>(d)});
        }

        return diagram;
    };

    if (r.persistence_pairs.size() > 0) {
        result.dim0 = convert(r.persistence_pairs[0]);
    }
    if (r.persistence_pairs.size() > 1) {
        result.dim1 = convert(r.persistence_pairs[1]);
    }
    if (r.persistence_pairs.size() > 2) {
        result.dim2 = convert(r.persistence_pairs[2]);
    }

    return result;
}

PersistenceResult
compute_persistence(const Eigen::MatrixXd& point_cloud, int max_dim, double threshold) {
    const auto N = point_cloud.rows();

    Eigen::MatrixXd dist(N, N);

    for (int i = 0; i < N; i++) {
        dist(i, i) = 0.0;

        for (int j = i + 1; j < N; j++) {
            double d = (point_cloud.row(i) - point_cloud.row(j)).norm();
            dist(i, j) = d;
            dist(j, i) = d;
        }
    }

    return compute_persistence_from_distances(dist, max_dim, threshold);
}

}  // namespace defect_gnn::topology