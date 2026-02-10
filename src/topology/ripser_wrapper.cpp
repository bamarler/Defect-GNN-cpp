#include "topology/ripser_wrapper.hpp"

#include <Eigen/Dense>

#define RIPSER_AS_LIBRARY
#include <ripser/ripser.cpp>  // NOLINT(bugprone-suspicious-include)
#include <vector>

namespace defect_gnn::topology {

PersistenceResult compute_persistence_from_distances(
    const Eigen::MatrixXd& distance_matrix,
    double threshold,  // NOLINT(bugprone-easily-swappable-parameters)
    unsigned num_threads) {
    const auto N = distance_matrix.rows();

    std::vector<value_t> distances;
    distances.reserve(N * (N - 1) / 2);

    for (int i = 1; i < N; i++) {
        for (int j = 0; j < i; j++) {
            distances.push_back(static_cast<value_t>(distance_matrix(i, j)));
        }
    }

    compressed_lower_distance_matrix dist(std::move(distances));

    auto thresh = static_cast<value_t>(threshold);
    coefficient_t modulus = 2;
    float ratio = 1.0F;

    ripser<sparse_distance_matrix> r(
        sparse_distance_matrix(dist, thresh), 2, thresh, ratio, modulus, num_threads);
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
compute_persistence(const Eigen::MatrixXd& point_cloud, double threshold, unsigned num_threads) {
    const auto N = point_cloud.rows();

    Eigen::VectorXd sq_norms = point_cloud.rowwise().squaredNorm();
    Eigen::MatrixXd sq_dist = sq_norms.replicate(1, N) + sq_norms.transpose().replicate(N, 1) -
                              2.0 * point_cloud * point_cloud.transpose();
    Eigen::MatrixXd dist = sq_dist.cwiseMax(0.0).cwiseSqrt();

    return compute_persistence_from_distances(dist, threshold, num_threads);
}

}  // namespace defect_gnn::topology