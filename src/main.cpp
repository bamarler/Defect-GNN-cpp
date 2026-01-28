#include "io/csv_parser.hpp"
#include "io/json_parser.hpp"
#include "io/vasp_parser.hpp"

#include <iostream>
#include <numeric>

int main() {
    auto embeddings = defect_gnn::io::parse_atom_embeddings("data/raw/atomic_embedding_CGCNN.json");

    // Get carbon (Z=6) embedding
    auto& carbon = embeddings[6];
    std::cout << "Carbon embedding size: " << carbon.size() << "\n";

    auto defects = defect_gnn::io::parse_defect_csv("data/raw/data.csv");
    std::cout << "Parsed csv with " << defects.size() << " defect entries.\n";

    auto vasp = defect_gnn::io::parse_vasp("data/raw/defective_structures/2_1.vasp");
    std::cout << "Parsed vasp with " << vasp.counts[0] << " " << vasp.elements[0]
              << " atoms and lattice: \n";
    for (int i = 0; i < 3; i++) {
        std::cout << vasp.lattice(i, 0) << " " << vasp.lattice(i, 1) << " " << vasp.lattice(i, 2)
                  << " \n\n";
    }

    for (int i = 0; i < std::reduce(vasp.counts.begin(), vasp.counts.end()); i++) {
        std::cout << vasp.frac_coords(i, 0) << " " << vasp.frac_coords(i, 1) << " "
                  << vasp.frac_coords(i, 2) << "\n";
    }

    return 0;
}