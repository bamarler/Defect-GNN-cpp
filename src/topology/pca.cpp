#include "topology/pca.hpp"

#include "Eigen/src/Core/util/Constants.h"
#include "Eigen/src/SVD/JacobiSVD.h"
#include "topology/betti_features.hpp"

#include <Eigen/Dense>

#include <fstream>
#include <ios>
#include <stdexcept>

namespace defect_gnn::topology {

void PCA::fit(const Eigen::MatrixXd& x, int n_components) {
    if (x.cols() != BETTI_FEATURE_DIM) {
        throw std::runtime_error("Inputted Matrix does not have the correct number of columns");
    }

    mean_ = x.colwise().mean();
    Eigen::MatrixXd centered = x.rowwise() - mean_.transpose();

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(centered, Eigen::ComputeThinV);

    components_ = svd.matrixV().leftCols(n_components);

    Eigen::VectorXd variance = svd.singularValues().array().square() / (x.rows() - 1);

    explained_var_ = variance.head(n_components) / variance.sum();

    n_components_ = n_components;

    fitted_ = true;
}

Eigen::MatrixXd PCA::transform(const Eigen::MatrixXd& x) const {
    if (!fitted_) {
        throw std::runtime_error("PCA::transform called before fit() or load()");
    }

    Eigen::MatrixXd centered = x.rowwise() - mean_.transpose();

    return centered * components_;
}

Eigen::MatrixXd PCA::fit_transform(const Eigen::MatrixXd& x, int n_components) {
    fit(x, n_components);

    return transform(x);
}

void PCA::save(const std::string& filepath) const {
    if (!fitted_) {
        throw std::runtime_error("PCA::save called before fit() or load()");
    }

    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }

    file.write(reinterpret_cast<const char*>(&n_components_), sizeof(n_components_));

    int mean_size = static_cast<int>(mean_.size());
    file.write(reinterpret_cast<const char*>(&mean_size), sizeof(mean_size));
    file.write(reinterpret_cast<const char*>(mean_.data()),
               static_cast<std::streamsize>(mean_size * sizeof(double)));

    int rows = static_cast<int>(components_.rows());
    int cols = static_cast<int>(components_.cols());
    file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
    file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
    file.write(reinterpret_cast<const char*>(components_.data()),
               static_cast<std::streamsize>(static_cast<size_t>(rows) * cols * sizeof(double)));

    int var_size = static_cast<int>(explained_var_.size());
    file.write(reinterpret_cast<const char*>(&var_size), sizeof(var_size));
    file.write(reinterpret_cast<const char*>(explained_var_.data()),
               static_cast<std::streamsize>(var_size * sizeof(double)));
}

void PCA::load(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }

    file.read(reinterpret_cast<char*>(&n_components_), sizeof(n_components_));

    int mean_size = 0;
    file.read(reinterpret_cast<char*>(&mean_size), sizeof(mean_size));
    mean_.resize(mean_size);
    file.read(reinterpret_cast<char*>(mean_.data()),
              static_cast<std::streamsize>(mean_size * sizeof(double)));

    int rows = 0;
    int cols = 0;
    file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    components_.resize(rows, cols);
    file.read(reinterpret_cast<char*>(components_.data()),
              static_cast<std::streamsize>(static_cast<size_t>(rows) * cols * sizeof(double)));

    int var_size = 0;
    file.read(reinterpret_cast<char*>(&var_size), sizeof(var_size));
    explained_var_.resize(var_size);
    file.read(reinterpret_cast<char*>(explained_var_.data()),
              static_cast<std::streamsize>(var_size * sizeof(double)));

    fitted_ = true;
}

}  // namespace defect_gnn::topology