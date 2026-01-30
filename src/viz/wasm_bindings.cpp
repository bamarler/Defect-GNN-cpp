#include "io/vasp_parser.hpp"
#include "viz/wasm_api.hpp"

#include <emscripten/bind.h>
#include <sstream>

namespace defect_gnn::viz {

namespace {

constexpr int LATTICE_ROWS = 3;
constexpr int LATTICE_COLS = 3;
constexpr int COORDS_PER_ATOM = 3;

double parse_scale_factor(std::istream& stream) {
    std::string line;
    std::getline(stream, line);
    const double kScale = std::stod(line);
    return kScale;
}

Eigen::Matrix3d parse_lattice_vectors(std::istream& stream, double scale) {
    Eigen::Matrix3d lattice;
    std::string line;

    for (int i = 0; i < LATTICE_ROWS; ++i) {
        std::getline(stream, line);
        std::istringstream lss(line);
        lss >> lattice(i, 0) >> lattice(i, 1) >> lattice(i, 2);
    }

    return lattice * scale;
}

std::vector<std::string> parse_element_symbols(std::istream& stream) {
    std::string line;
    std::getline(stream, line);
    std::istringstream ess(line);

    std::vector<std::string> elements;
    std::string elem;
    while (ess >> elem) {
        elements.push_back(elem);
    }

    return elements;
}

std::pair<std::vector<int>, int> parse_element_counts(std::istream& stream) {
    std::string line;
    std::getline(stream, line);
    std::istringstream css(line);

    std::vector<int> counts;
    int total = 0;
    int count = 0;

    while (css >> count) {
        counts.push_back(count);
        total += count;
    }

    return {counts, total};
}

void parse_atom_positions(std::istream& stream,
                          int total_atoms,
                          const std::vector<int>& counts,
                          Eigen::MatrixXd& frac_coords,
                          std::vector<int>& atom_types) {
    std::string line;

    // Skip coordinate type line (Direct/Cartesian)
    std::getline(stream, line);

    frac_coords.resize(total_atoms, COORDS_PER_ATOM);
    atom_types.resize(static_cast<size_t>(total_atoms));

    int atom_idx = 0;
    for (size_t type = 0; type < counts.size(); ++type) {
        for (int i = 0; i < counts[type]; ++i) {
            std::getline(stream, line);
            std::istringstream pss(line);
            pss >> frac_coords(atom_idx, 0) >> frac_coords(atom_idx, 1) >> frac_coords(atom_idx, 2);
            atom_types[static_cast<size_t>(atom_idx)] = static_cast<int>(type);
            ++atom_idx;
        }
    }
}

io::VASPStructure parse_vasp_string(const std::string& content) {
    io::VASPStructure result;
    std::istringstream stream(content);
    std::string line;

    // Line 1: comment (skip)
    std::getline(stream, line);

    // Line 2: scale factor
    const double kScale = parse_scale_factor(stream);

    // Lines 3-5: lattice vectors
    result.lattice = parse_lattice_vectors(stream, kScale);

    // Line 6: element symbols
    result.elements = parse_element_symbols(stream);

    // Line 7: element counts
    auto [counts, total] = parse_element_counts(stream);
    result.counts = counts;

    // Remaining lines: atom positions
    parse_atom_positions(stream, total, result.counts, result.frac_coords, result.atom_types);

    return result;
}

}  // namespace

bool WasmAPI::load_structure(const std::string& vasp_content) {
    try {
        vasp_ = std::make_unique<io::VASPStructure>(parse_vasp_string(vasp_content));
        structure_ = std::make_unique<crystal::Structure>(*vasp_);
        neighbors_.reset();
        return true;
    } catch (const std::exception& /*unused*/) {
        return false;
    }
}

void WasmAPI::build_graph(double r_cutoff, size_t max_neighbors) {
    if (!structure_) {
        return;
    }
    neighbors_ = std::make_unique<graph::NeighborList>(*structure_, r_cutoff, max_neighbors);
}

// === Structure accessors ===

size_t WasmAPI::num_atoms() const {
    if (!structure_) {
        return 0;
    }
    return structure_->num_atoms();
}

std::vector<float> WasmAPI::get_positions() const {
    if (!structure_) {
        return {};
    }

    const auto& atoms = structure_->atoms();
    std::vector<float> positions;
    positions.reserve(static_cast<size_t>(atoms.size()) * COORDS_PER_ATOM);

    for (const auto& atom : atoms) {
        positions.push_back(static_cast<float>(atom.position.x()));
        positions.push_back(static_cast<float>(atom.position.y()));
        positions.push_back(static_cast<float>(atom.position.z()));
    }

    return positions;
}

std::vector<int> WasmAPI::get_atom_types() const {
    if (!vasp_) {
        return {};
    }
    return vasp_->atom_types;
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
    if (!structure_) {
        return {};
    }

    const auto& lattice = structure_->lattice();
    std::vector<float> result;
    result.reserve(LATTICE_ROWS * LATTICE_COLS);

    for (int i = 0; i < LATTICE_ROWS; ++i) {
        for (int j = 0; j < LATTICE_COLS; ++j) {
            result.push_back(static_cast<float>(lattice(i, j)));
        }
    }

    return result;
}

// === Graph accessors ===

size_t WasmAPI::num_edges() const {
    if (!neighbors_ || !structure_) {
        return 0;
    }

    size_t count = 0;
    const size_t kNumAtoms = structure_->num_atoms();

    for (size_t i = 0; i < kNumAtoms; ++i) {
        count += neighbors_->neighbors(i).size();
    }

    return count;
}

std::vector<int> WasmAPI::get_edge_sources() const {
    if (!neighbors_ || !structure_) {
        return {};
    }

    std::vector<int> sources;
    sources.reserve(num_edges());
    const size_t kNumAtoms = structure_->num_atoms();

    for (size_t i = 0; i < kNumAtoms; ++i) {
        const auto& nbrs = neighbors_->neighbors(i);
        for (size_t j = 0; j < nbrs.size(); ++j) {
            sources.push_back(static_cast<int>(i));
        }
    }

    return sources;
}

std::vector<int> WasmAPI::get_edge_targets() const {
    if (!neighbors_ || !structure_) {
        return {};
    }

    std::vector<int> targets;
    targets.reserve(num_edges());
    const size_t kNumAtoms = structure_->num_atoms();

    for (size_t i = 0; i < kNumAtoms; ++i) {
        for (const auto& neighbor : neighbors_->neighbors(i)) {
            targets.push_back(static_cast<int>(neighbor.idx));
        }
    }

    return targets;
}

std::vector<float> WasmAPI::get_edge_distances() const {
    if (!neighbors_ || !structure_) {
        return {};
    }

    std::vector<float> distances;
    distances.reserve(num_edges());
    const size_t kNumAtoms = structure_->num_atoms();

    for (size_t i = 0; i < kNumAtoms; ++i) {
        for (const auto& neighbor : neighbors_->neighbors(i)) {
            distances.push_back(static_cast<float>(neighbor.distance));
        }
    }

    return distances;
}

std::vector<float> WasmAPI::get_edge_displacements() const {
    if (!neighbors_ || !structure_) {
        return {};
    }

    std::vector<float> displacements;
    displacements.reserve(static_cast<size_t>(num_edges()) * COORDS_PER_ATOM);
    const size_t kNumAtoms = structure_->num_atoms();

    for (size_t i = 0; i < kNumAtoms; ++i) {
        for (const auto& neighbor : neighbors_->neighbors(i)) {
            displacements.push_back(static_cast<float>(neighbor.displacement.x()));
            displacements.push_back(static_cast<float>(neighbor.displacement.y()));
            displacements.push_back(static_cast<float>(neighbor.displacement.z()));
        }
    }

    return displacements;
}

}  // namespace defect_gnn::viz

EMSCRIPTEN_BINDINGS(defect_gnn_viz) {
    using namespace defect_gnn::viz;

    emscripten::class_<WasmAPI>("WasmAPI")
        .constructor<>()
        .function("loadStructure", &WasmAPI::load_structure)
        .function("buildGraph", &WasmAPI::build_graph)
        // Structure
        .function("numAtoms", &WasmAPI::num_atoms)
        .function("getPositions", &WasmAPI::get_positions)
        .function("getAtomTypes", &WasmAPI::get_atom_types)
        .function("getElements", &WasmAPI::get_elements)
        .function("getElementCounts", &WasmAPI::get_element_counts)
        .function("getLatticeVectors", &WasmAPI::get_lattice_vectors)
        // Graph
        .function("numEdges", &WasmAPI::num_edges)
        .function("getEdgeSources", &WasmAPI::get_edge_sources)
        .function("getEdgeTargets", &WasmAPI::get_edge_targets)
        .function("getEdgeDistances", &WasmAPI::get_edge_distances)
        .function("getEdgeDisplacements", &WasmAPI::get_edge_displacements);

    emscripten::register_vector<float>("VectorFloat");
    emscripten::register_vector<int>("VectorInt");
    emscripten::register_vector<std::string>("VectorString");
}