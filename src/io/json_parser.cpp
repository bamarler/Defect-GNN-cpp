#include "io/json_parser.hpp"

#include <Eigen/Dense>
#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace defect_gnn::io {

std::map<int, Eigen::VectorXd> parse_atom_embeddings(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    nlohmann::json data;
    file >> data;

    std::map<int, Eigen::VectorXd> embeddings;

    for (const auto& [key, value] : data.items()) {
        int atomic_num = std::stoi(key);

        std::vector<double> vec = value.get<std::vector<double>>();

        Eigen::VectorXd eigen_vec =
            Eigen::Map<Eigen::VectorXd>(vec.data(), static_cast<Eigen::Index>(vec.size()));

        embeddings[atomic_num] = eigen_vec;
    }

    return embeddings;
}

}  // namespace defect_gnn::io