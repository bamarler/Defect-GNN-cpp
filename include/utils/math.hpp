#pragma once

#include <Eigen/Dense>

#include <cmath>

namespace defect_gnn::utils {

inline double mean(const Eigen::VectorXd& v) {
    return v.mean();
}

inline double std(const Eigen::VectorXd& v) {
    double m = v.mean();
    return std::sqrt((v.array() - m).square().mean());
}

inline double max(const Eigen::VectorXd& v) {
    return v.maxCoeff();
}

inline double min(const Eigen::VectorXd& v) {
    return v.minCoeff();
}

inline double weighted_sum(const Eigen::VectorXd& v, double weight) {
    return v.sum() * weight;
}

inline Eigen::VectorXd arange(double start, double end, double step) {
    int n = static_cast<int>(std::ceil((end - start) / step));
    return Eigen::VectorXd::LinSpaced(n, start, start + (n - 1) * step);
}

}  // namespace defect_gnn::utils