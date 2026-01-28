#pragma once

#include "io/vasp_parser.hpp"

#include <Eigen/Dense>

#include <vector>

namespace defect_gnn::crystal {

class Atom {
public:
    int element;
    Eigen::Vector3d position;
    Eigen::Vector3d frac_position;
};

class Structure {
public:
    explicit Structure(const io::VASPStructure& vasp);

    [[nodiscard]] const Eigen::Matrix3d& lattice() const;
    [[nodiscard]] const std::vector<Atom>& atoms() const;
    [[nodiscard]] size_t num_atoms() const;

    [[nodiscard]] double distance(size_t i, size_t j) const;
    [[nodiscard]] Eigen::Vector3d displacement(size_t i, size_t j) const;

private:
    Eigen::Matrix3d lattice_;
    Eigen::Matrix3d inv_lattice_;
    std::vector<Atom> atoms_;
};
}  // namespace defect_gnn::crystal