#pragma once

#include <Eigen/Dense>

#include <string>

namespace defect_gnn::topology {

class PCA {
public:
    void fit(const Eigen::MatrixXd& x, int n_components = 6);

    [[nodiscard]] Eigen::MatrixXd transform(const Eigen::MatrixXd& x) const;

    Eigen::MatrixXd fit_transform(const Eigen::MatrixXd& x, int n_components = 6);

    void save(const std::string& path) const;
    void load(const std::string& path);

    [[nodiscard]] const int& n_components() const { return n_components_; };
    [[nodiscard]] const Eigen::VectorXd& mean() const { return mean_; };
    [[nodiscard]] const Eigen::MatrixXd& components() const { return components_; };
    [[nodiscard]] const Eigen::VectorXd& explained_variance_ratio() const {
        return explained_var_;
    };

private:
    bool fitted_ = false;

    int n_components_ = 0;

    Eigen::VectorXd mean_;
    Eigen::MatrixXd components_;
    Eigen::VectorXd explained_var_;
};

}  // namespace defect_gnn::topology