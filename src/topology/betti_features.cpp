#include "topology/betti_features.hpp"

#include "crystal/structure.hpp"
#include "topology/ripser_wrapper.hpp"
#include "utils/math.hpp"

#include <Eigen/Dense>

#include <cmath>
#include <cstddef>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <vector>

namespace defect_gnn::topology {

BettiStatistics compute_statistics(const PersistenceDiagram& diagram,
                                   const std::string& values_type,
                                   double weight) {
    std::vector<double> values;

    for (PersistencePair pair : diagram) {
        if (pair.death == INFINITY) {
            continue;
        }

        if (values_type == "birth") {
            values.push_back(pair.birth);
        } else if (values_type == "death") {
            values.push_back(pair.death);
        } else if (values_type == "persistence") {
            values.push_back(persistence(pair));
        }
    }

    if (values.size() == 0) {
        return {};
    }

    Eigen::VectorXd eigen_values =
        Eigen::Map<Eigen::VectorXd>(values.data(), static_cast<int>(values.size()));

    return {utils::mean(eigen_values),
            utils::std(eigen_values),
            utils::max(eigen_values),
            utils::min(eigen_values),
            utils::weighted_sum(eigen_values, weight)};
}

Eigen::VectorXd
compute_atom_betti_features(const crystal::Structure& structure, size_t atom_idx, double r_cutoff) {
    auto center_element = structure.atoms()[atom_idx];
    int element_count = structure.count(center_element.element);

    std::vector<Eigen::Vector3d> points;
    points.push_back(center_element.position);

    for (size_t j = 0; j < structure.num_atoms(); j++) {
        if (j == atom_idx) {
            continue;
        }

        if (structure.distance(atom_idx, j) < r_cutoff) {
            auto displacement = structure.displacement(atom_idx, j);

            points.emplace_back(center_element.position + displacement);
        }
    }

    Eigen::MatrixXd point_cloud(points.size(), 3);

    for (size_t i = 0; i < points.size(); i++) {
        point_cloud.row(static_cast<int>(i)) = points[i].transpose();
    }

    PersistenceResult result = topology::compute_persistence(point_cloud, r_cutoff);

    double weight = 1.0 / element_count;

    Eigen::VectorXd atom_features(BETTI_FEATURE_DIM);

    int idx = 0;
    auto append = [&](const BettiStatistics& s) {
        atom_features.segment(idx, 5) << s.mean, s.std, s.max, s.min, s.weighted_sum;
        idx += 5;
    };

    // 5 Features for dim0 from death only
    append(compute_statistics(result.dim0, "death", weight));

    // 15 features for dim1 from persistence, birth, and death
    for (const std::string& type : {"persistence", "birth", "death"}) {
        append(compute_statistics(result.dim1, type, weight));
    }

    // 15 features for dim2 from persistence, birth, and death
    for (const std::string& type : {"persistence", "birth", "death"}) {
        append(compute_statistics(result.dim2, type, weight));
    }

    return atom_features;
}

Eigen::MatrixXd compute_structure_betti_features(const crystal::Structure& structure,
                                                 double r_cutoff) {
    Eigen::MatrixXd structure_features(structure.num_atoms(), BETTI_FEATURE_DIM);

    for (int i = 0; i < static_cast<int>(structure.num_atoms()); i++) {
        structure_features.row(i) = compute_atom_betti_features(structure, i, r_cutoff);
    }

    return structure_features;
}

void save_betti_features(const std::string& filepath, const Eigen::MatrixXd& features) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }

    int rows = static_cast<int>(features.rows());
    int cols = static_cast<int>(features.cols());

    file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
    file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
    file.write(reinterpret_cast<const char*>(features.data()),
               static_cast<std::streamsize>(static_cast<size_t>(rows) * cols * sizeof(double)));
}

Eigen::MatrixXd load_betti_features(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }

    int rows = 0;
    int cols = 0;

    file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    file.read(reinterpret_cast<char*>(&cols), sizeof(cols));

    Eigen::MatrixXd features(rows, cols);
    file.read(reinterpret_cast<char*>(features.data()),
              static_cast<std::streamsize>(static_cast<size_t>(rows) * cols * sizeof(double)));

    return features;
}

}  // namespace defect_gnn::topology