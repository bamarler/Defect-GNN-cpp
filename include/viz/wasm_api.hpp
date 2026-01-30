#pragma once

#include "crystal/structure.hpp"
#include "graph/neighbor_list.hpp"
#include "io/vasp_parser.hpp"

#include <memory>
#include <string>
#include <vector>

namespace defect_gnn::viz {

class WasmAPI {
public:
    WasmAPI() = default;

    bool load_structure(const std::string& vasp_content);
    void build_graph(double r_cutoff, size_t max_neighbors);

    // === Structure accessors (unit cell) ===
    [[nodiscard]] size_t num_atoms() const;
    [[nodiscard]] std::vector<float> get_positions() const;
    [[nodiscard]] std::vector<int> get_atom_types() const;
    [[nodiscard]] std::vector<std::string> get_elements() const;
    [[nodiscard]] std::vector<int> get_element_counts() const;
    [[nodiscard]] std::vector<float> get_lattice_vectors() const;

    // === Graph accessors (edges) ===
    [[nodiscard]] size_t num_edges() const;
    [[nodiscard]] std::vector<int> get_edge_sources() const;
    [[nodiscard]] std::vector<int> get_edge_targets() const;
    [[nodiscard]] std::vector<float> get_edge_distances() const;
    [[nodiscard]] std::vector<float> get_edge_displacements() const;

private:
    std::unique_ptr<io::VASPStructure> vasp_;
    std::unique_ptr<crystal::Structure> structure_;
    std::unique_ptr<graph::NeighborList> neighbors_;
};

}  // namespace defect_gnn::viz