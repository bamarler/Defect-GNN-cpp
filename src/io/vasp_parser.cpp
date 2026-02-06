#include "io/vasp_parser.hpp"

#include <Eigen/Dense>

#include <fstream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace defect_gnn::io {

VASPStructure parse_vasp(const std::string& filepath) {  // NOLINT(readability-function-size)
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    std::vector<std::string> lines;

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    VASPStructure vasp;

    std::stringstream scale_ss(lines[1]);
    double scale_factor = 1.0;
    scale_ss >> scale_factor;

    for (int i = 0; i < 3; i++) {
        std::stringstream iss(lines[i + 2]);

        iss >> vasp.lattice(i, 0) >> vasp.lattice(i, 1) >> vasp.lattice(i, 2);
    }

    vasp.lattice *= scale_factor;

    std::stringstream elem_ss(lines[5]);
    std::string elem;
    while (elem_ss >> elem) {
        vasp.elements.push_back(elem);
    }

    std::stringstream count_ss(lines[6]);
    int count = -1;
    while (count_ss >> count) {
        vasp.counts.push_back(count);
    }

    bool is_direct = lines[7][0] == 'd' || lines[7][0] == 'D';

    int total_atoms = std::reduce(vasp.counts.begin(), vasp.counts.end());

    vasp.frac_coords.resize(total_atoms, 3);
    vasp.atom_types.resize(total_atoms);

    int atom_idx = 0;
    for (int elem_idx = 0; elem_idx < static_cast<int>(vasp.counts.size()); ++elem_idx) {
        for (int j = 0; j < vasp.counts[elem_idx]; ++j) {
            std::stringstream atom_ss(lines[atom_idx + 8]);

            atom_ss >> vasp.frac_coords(atom_idx, 0) >> vasp.frac_coords(atom_idx, 1) >>
                vasp.frac_coords(atom_idx, 2);
            vasp.atom_types[atom_idx] = elem_idx;

            ++atom_idx;
        }
    }

    if (!is_direct) {
        Eigen::Matrix3d inv_lattice = vasp.lattice.inverse();
        vasp.frac_coords = vasp.frac_coords * inv_lattice;
    }

    return vasp;
}

}  // namespace defect_gnn::io