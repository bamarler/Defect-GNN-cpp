#include "viz/wasm_api.hpp"

#include <sstream>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
    #include <emscripten/bind.h>
#endif

namespace defect_gnn::viz {

namespace {

std::vector<std::string> split_lines(const std::string& content) {
    std::istringstream stream(content);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

void parse_lattice(io::VASPStructure& vasp, const std::vector<std::string>& lines) {
    std::stringstream scale_ss(lines[1]);
    double scale_factor = 1.0;
    scale_ss >> scale_factor;

    for (int i = 0; i < 3; i++) {
        std::stringstream iss(lines[i + 2]);
        iss >> vasp.lattice(i, 0) >> vasp.lattice(i, 1) >> vasp.lattice(i, 2);
    }
    vasp.lattice *= scale_factor;
}

void parse_elements(io::VASPStructure& vasp, const std::vector<std::string>& lines) {
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
}

void parse_coordinates(io::VASPStructure& vasp, const std::vector<std::string>& lines) {
    bool is_direct = lines[7][0] == 'd' || lines[7][0] == 'D';

    int total_atoms = 0;
    for (int c : vasp.counts) {
        total_atoms += c;
    }

    vasp.frac_coords.resize(total_atoms, 3);
    vasp.atom_types.resize(total_atoms);

    int atom_idx = 0;
    for (int elem_idx = 0; elem_idx < static_cast<int>(vasp.counts.size()); ++elem_idx) {
        for (int j = 0; j < vasp.counts[elem_idx]; ++j) {
            if (atom_idx + 8 >= static_cast<int>(lines.size())) {
                throw std::runtime_error("Invalid VASP file: missing atom coordinates");
            }
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
}

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// VASP String Parser
// ─────────────────────────────────────────────────────────────────────────────

io::VASPStructure parse_vasp_string(const std::string& content) {
    std::vector<std::string> lines = split_lines(content);

    if (lines.size() < 8) {
        throw std::runtime_error("Invalid VASP file: too few lines");
    }

    io::VASPStructure vasp;
    parse_lattice(vasp, lines);
    parse_elements(vasp, lines);
    parse_coordinates(vasp, lines);

    return vasp;
}

// ─────────────────────────────────────────────────────────────────────────────
// WasmAPI Implementation
// ─────────────────────────────────────────────────────────────────────────────

bool WasmAPI::load_structure(const std::string& vasp_content) {
    try {
        vasp_ = std::make_unique<io::VASPStructure>(parse_vasp_string(vasp_content));
        structure_ = std::make_unique<crystal::Structure>(*vasp_);
        neighbors_.reset();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void WasmAPI::build_graph(double r_cutoff, size_t max_neighbors) {
    if (!structure_) {
        throw std::runtime_error("No structure loaded. Call load_structure() first.");
    }
    neighbors_ = std::make_unique<graph::NeighborList>(*structure_, r_cutoff, max_neighbors);
}

size_t WasmAPI::num_atoms() const {
    if (structure_) {
        return structure_->num_atoms();
    }
    return 0;
}

std::vector<float> WasmAPI::get_positions() const {
    std::vector<float> positions;
    if (!structure_) {
        return positions;
    }

    positions.reserve(structure_->num_atoms() * 3);
    for (const auto& atom : structure_->atoms()) {
        positions.push_back(static_cast<float>(atom.position.x()));
        positions.push_back(static_cast<float>(atom.position.y()));
        positions.push_back(static_cast<float>(atom.position.z()));
    }
    return positions;
}

std::vector<int> WasmAPI::get_atom_types() const {
    std::vector<int> types;
    if (!structure_) {
        return types;
    }

    types.reserve(structure_->num_atoms());
    for (const auto& atom : structure_->atoms()) {
        types.push_back(atom.element);
    }
    return types;
}

std::vector<std::string> WasmAPI::get_elements() const {
    if (!vasp_) {
        return {};
    }
    return vasp_->elements;
}

std::vector<int> WasmAPI::get_element_counts() const {
    if (!vasp_) {
        return {};
    }
    return vasp_->counts;
}

std::vector<float> WasmAPI::get_lattice_vectors() const {
    std::vector<float> lattice;
    if (!structure_) {
        return lattice;
    }

    lattice.reserve(9);
    const auto& lat = structure_->lattice();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            lattice.push_back(static_cast<float>(lat(i, j)));
        }
    }
    return lattice;
}

size_t WasmAPI::num_edges() const {
    if (!neighbors_) {
        return 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < structure_->num_atoms(); i++) {
        count += neighbors_->neighbors(i).size();
    }
    return count;
}

std::vector<int> WasmAPI::get_edge_sources() const {
    std::vector<int> sources;
    if (!neighbors_ || !structure_) {
        return sources;
    }

    sources.reserve(num_edges());
    for (size_t i = 0; i < structure_->num_atoms(); i++) {
        for (size_t j = 0; j < neighbors_->neighbors(i).size(); j++) {
            sources.push_back(static_cast<int>(i));
        }
    }
    return sources;
}

std::vector<int> WasmAPI::get_edge_targets() const {
    std::vector<int> targets;
    if (!neighbors_ || !structure_) {
        return targets;
    }

    targets.reserve(num_edges());
    for (size_t i = 0; i < structure_->num_atoms(); i++) {
        for (const auto& neighbor : neighbors_->neighbors(i)) {
            targets.push_back(static_cast<int>(neighbor.idx));
        }
    }
    return targets;
}

std::vector<float> WasmAPI::get_edge_distances() const {
    std::vector<float> distances;
    if (!neighbors_ || !structure_) {
        return distances;
    }

    distances.reserve(num_edges());
    for (size_t i = 0; i < structure_->num_atoms(); i++) {
        for (const auto& neighbor : neighbors_->neighbors(i)) {
            distances.push_back(static_cast<float>(neighbor.distance));
        }
    }
    return distances;
}

}  // namespace defect_gnn::viz

// ─────────────────────────────────────────────────────────────────────────────
// Emscripten Bindings
// ─────────────────────────────────────────────────────────────────────────────

#ifdef __EMSCRIPTEN__

using namespace defect_gnn::viz;

EMSCRIPTEN_BINDINGS(defect_gnn_viz) {
    emscripten::class_<WasmAPI>("WasmAPI")
        .constructor<>()
        .function("loadStructure", &WasmAPI::load_structure)
        .function("buildGraph", &WasmAPI::build_graph)
        .function("numAtoms", &WasmAPI::num_atoms)
        .function("getPositions", &WasmAPI::get_positions)
        .function("getAtomTypes", &WasmAPI::get_atom_types)
        .function("getElements", &WasmAPI::get_elements)
        .function("getElementCounts", &WasmAPI::get_element_counts)
        .function("getLatticeVectors", &WasmAPI::get_lattice_vectors)
        .function("numEdges", &WasmAPI::num_edges)
        .function("getEdgeSources", &WasmAPI::get_edge_sources)
        .function("getEdgeTargets", &WasmAPI::get_edge_targets)
        .function("getEdgeDistances", &WasmAPI::get_edge_distances);

    emscripten::register_vector<float>("VectorFloat");
    emscripten::register_vector<int>("VectorInt");
    emscripten::register_vector<std::string>("VectorString");
}

#endif