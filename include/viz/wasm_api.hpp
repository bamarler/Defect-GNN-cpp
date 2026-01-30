#pragma once

#include "crystal/structure.hpp"
#include "graph/neighbor_list.hpp"
#include "io/vasp_parser.hpp"

#include <memory>
#include <string>
#include <vector>

namespace defect_gnn::viz {

/// Holds state for the WASM visualization API.
class WasmAPI {
public:
    WasmAPI() = default;

    /// Parse a VASP file from string content.
    /// Returns true on success, false on parse error.
    bool load_structure(const std::string& vasp_content);

    /// Build neighbor list with given parameters.
    void build_graph(double r_cutoff, size_t max_neighbors);

    // ─────────────────────────────────────────────────────────────
    // Structure Data Accessors
    // ─────────────────────────────────────────────────────────────

    [[nodiscard]] size_t num_atoms() const;
    [[nodiscard]] std::vector<float> get_positions() const;
    [[nodiscard]] std::vector<int> get_atom_types() const;
    [[nodiscard]] std::vector<std::string> get_elements() const;
    [[nodiscard]] std::vector<int> get_element_counts() const;
    [[nodiscard]] std::vector<float> get_lattice_vectors() const;

    // ─────────────────────────────────────────────────────────────
    // Graph Data Accessors
    // ─────────────────────────────────────────────────────────────

    [[nodiscard]] size_t num_edges() const;
    [[nodiscard]] std::vector<int> get_edge_sources() const;
    [[nodiscard]] std::vector<int> get_edge_targets() const;
    [[nodiscard]] std::vector<float> get_edge_distances() const;

private:
    std::unique_ptr<io::VASPStructure> vasp_;
    std::unique_ptr<crystal::Structure> structure_;
    std::unique_ptr<graph::NeighborList> neighbors_;
};

/// Parse VASP content from string instead of file.
[[nodiscard]] io::VASPStructure parse_vasp_string(const std::string& content);

}  // namespace defect_gnn::viz