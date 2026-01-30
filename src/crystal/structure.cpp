#include "crystal/structure.hpp"

#include "io/vasp_parser.hpp"

namespace defect_gnn::crystal {

Structure::Structure(const io::VASPStructure& vasp)
    : lattice_(vasp.lattice), inv_lattice_(vasp.lattice.inverse()) {
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

}  // namespace defect_gnn::crystal