#pragma once

#include <Eigen/Dense>

namespace defect_gnn::topology {

class PCA {
public:
    void fit(const Eigen::MatrixXd& x, int n_components = 6);

    [[nodiscard]] Eigen::MatrixXd transform(const Eigen::MatrixXd& x) const;

    void save(const std::string& path) const;
    void load(const std::string& path);

    [[nodiscard]] Eigen::VectorXd explained_variance_ratio() const;

private:
    Eigen::MatrixXd components_;
    Eigen::VectorXd mean_;
    Eigen::VectorXd explained_var_;
};

}  // namespace defect_gnn::topology