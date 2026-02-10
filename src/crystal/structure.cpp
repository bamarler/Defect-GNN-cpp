#include "crystal/structure.hpp"

#include "io/vasp_parser.hpp"

namespace defect_gnn::crystal {

Structure::Structure(const io::VASPStructure& vasp)
    : lattice_(vasp.lattice), inv_lattice_(vasp.lattice.inverse()) {
    for (size_t elem_idx = 0; elem_idx < vasp.counts.size(); elem_idx++) {
        counts_[static_cast<int>(elem_idx)] = vasp.counts[elem_idx];
    }

    for (int i = 0; i < vasp.frac_coords.rows(); i++) {
        Atom atom;
        atom.element = vasp.atom_types[i];
        atom.frac_position = vasp.frac_coords.row(i);
        atom.position = lattice_.transpose() * atom.frac_position;
        atoms_.push_back(atom);
    }
}

const Eigen::Matrix3d& Structure::lattice() const {
    return lattice_;
}

const std::vector<Atom>& Structure::atoms() const {
    return atoms_;
}

size_t Structure::num_atoms() const {
    return atoms_.size();
}

double Structure::distance(size_t i, size_t j) const {
    return displacement(i, j).norm();
}

Eigen::Vector3d Structure::displacement(size_t i, size_t j) const {
    Eigen::Vector3d delta_frac = atoms_[j].frac_position - atoms_[i].frac_position;

    for (int k = 0; k < 3; k++) {
        delta_frac[k] -= std::round(delta_frac[k]);
    }

    return lattice_.transpose() * delta_frac;
}

int Structure::count(int element) const {
    return counts_.at(element);
}

Eigen::MatrixXd Structure::compute_distance_matrix() const {
    auto N = static_cast<Eigen::Index>(atoms_.size());
    Eigen::MatrixXd distances(N, N);

    for (Eigen::Index i = 0; i < N; i++) {
        distances(i, i) = 0.0;
        for (Eigen::Index j = i + 1; j < N; j++) {
            double d = distance(static_cast<size_t>(i), static_cast<size_t>(j));
            distances(i, j) = d;
            distances(j, i) = d;
        }
    }

    return distances;
}

}  // namespace defect_gnn::crystal