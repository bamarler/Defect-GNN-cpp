# Defect_GNN C++ Recreation: Project Roadmap

## Overview

Recreate the paper "Leveraging Persistent Homology Features for Accurate Defect Formation Energy Predictions via Graph Neural Networks" (Fang & Yan, Chem. Mater. 2025) in C++, with eventual WebAssembly compilation for browser-based visualization.

**Paper claims to reproduce:**
- 55% MAE reduction with persistent homology features
- Global max pooling outperforms mean pooling for defect tasks
- Transformer architecture performs best (MAE: 0.72 eV)

---

## Phase 0: Setup & Data Acquisition

### Step 0.1: Fork Repository
```bash
# Fork https://github.com/qmatyanlab/Defect_GNN
# Clone your fork
git clone https://github.com/<your-username>/Defect_GNN.git
cd Defect_GNN

# Extract dataset
cd datasets
tar -xzf raw.tar.gz
```

**What you get:**
- `data.csv` — Labels (pristine_idx, vacancy_idx, formation_energy)
- `atomic_embedding_CGCNN.json` — 92-dim embeddings per element
- `pristine_structures/` — 1112 VASP files
- `defective_structures/` — 7753 VASP files

### Step 0.2: Also Clone Configurational-Disorder (Reference Code)
```bash
git clone https://github.com/qmatyanlab/Configurational-Disorder.git
```

**Files to reference:**
| File | Purpose |
|------|---------|
| `Disorder_GNN/model/model_embedding.py` | CGNN, GAT, Transformer architectures |
| `Disorder_GNN/GNN.py` | Training loop, Optuna hyperparameter search |
| `Disorder_GNN/dataset.py` | PyG dataset class pattern |
| `Disorder_GNN/utilities.py` | Graph construction |

### Step 0.3: Create Your C++ Project Structure
```
defect-gnn-cpp/
│
├── CMakeLists.txt
├── Makefile                      # Convenience wrapper
├── README.md
│
├── data/                         # Symlink or copy from fork
│   ├── raw/
│   │   ├── data.csv
│   │   ├── atomic_embedding_CGCNN.json
│   │   ├── pristine_structures/
│   │   └── defective_structures/
│   └── processed/                # Generated at runtime
│       ├── graphs/               # Serialized crystal graphs
│       └── betti/                # Precomputed topology features
│
├── include/
│   ├── io/
│   │   ├── vasp_parser.hpp
│   │   ├── csv_parser.hpp
│   │   └── json_parser.hpp
│   │
│   ├── crystal/
│   │   ├── structure.hpp
│   │   ├── lattice.hpp
│   │   └── atom.hpp
│   │
│   ├── graph/
│   │   ├── crystal_graph.hpp
│   │   ├── neighbor_list.hpp
│   │   └── edge_features.hpp
│   │
│   ├── topology/
│   │   ├── persistent_homology.hpp
│   │   ├── betti_features.hpp
│   │   └── pca.hpp
│   │
│   ├── nn/
│   │   ├── tensor.hpp            # Or use Eigen
│   │   ├── linear.hpp
│   │   ├── activation.hpp
│   │   ├── conv/
│   │   │   ├── message_passing.hpp
│   │   │   ├── cgconv.hpp
│   │   │   ├── gatv2conv.hpp
│   │   │   └── transformer_conv.hpp
│   │   ├── pool/
│   │   │   ├── global_max_pool.hpp
│   │   │   └── global_mean_pool.hpp
│   │   └── model.hpp
│   │
│   ├── train/
│   │   ├── dataset.hpp
│   │   ├── dataloader.hpp
│   │   ├── optimizer.hpp
│   │   └── trainer.hpp
│   │
│   └── utils/
│       ├── math.hpp
│       └── logging.hpp
│
├── src/
│   ├── io/
│   │   ├── vasp_parser.cpp
│   │   ├── csv_parser.cpp
│   │   └── json_parser.cpp
│   │
│   ├── crystal/
│   │   ├── structure.cpp
│   │   └── lattice.cpp
│   │
│   ├── graph/
│   │   ├── crystal_graph.cpp
│   │   ├── neighbor_list.cpp
│   │   └── edge_features.cpp
│   │
│   ├── topology/
│   │   ├── persistent_homology.cpp
│   │   ├── betti_features.cpp
│   │   └── pca.cpp
│   │
│   ├── nn/
│   │   ├── linear.cpp
│   │   ├── conv/
│   │   │   ├── message_passing.cpp
│   │   │   ├── cgconv.cpp
│   │   │   ├── gatv2conv.cpp
│   │   │   └── transformer_conv.cpp
│   │   ├── pool/
│   │   │   └── global_pool.cpp
│   │   └── model.cpp
│   │
│   ├── train/
│   │   ├── dataset.cpp
│   │   ├── dataloader.cpp
│   │   ├── optimizer.cpp
│   │   └── trainer.cpp
│   │
│   └── main.cpp
│
├── tests/
│   ├── test_vasp_parser.cpp
│   ├── test_graph_construction.cpp
│   ├── test_betti_features.cpp
│   ├── test_convolutions.cpp
│   └── test_forward_pass.cpp
│
├── scripts/
│   ├── precompute_betti.py       # Use Python ripser initially
│   └── validate_against_pytorch.py
│
└── third_party/
    ├── eigen/                    # Header-only
    ├── nlohmann_json/            # Header-only
    ├── nanoflann/                # Header-only (KD-tree)
    └── ripser/                   # C++ persistent homology
```

---

## Phase 1: Data Pipeline (Weeks 1-2)

### 1.1 VASP Parser

**Reference:** `Defect_GNN/Betti_number.py:15-46` (`get_prim_structure_info`)

**File:** `include/io/vasp_parser.hpp`
```cpp
namespace defect_gnn::io {

struct VASPStructure {
    Eigen::Matrix3d lattice;           // 3x3 lattice vectors (rows)
    std::vector<std::string> elements; // Element symbols
    std::vector<int> counts;           // Count per element
    Eigen::MatrixXd frac_coords;       // Nx3 fractional coordinates
    std::vector<int> atom_types;       // Element index per atom
};

// Parse VASP POSCAR/CONTCAR format
// Reference: Betti_number.py:15-46
VASPStructure parse_vasp(const std::string& filepath);

// Convert fractional to Cartesian coordinates
Eigen::MatrixXd frac_to_cart(const Eigen::Matrix3d& lattice,
                              const Eigen::MatrixXd& frac_coords);

}  // namespace defect_gnn::io
```

**VASP format reminder:**
```
Line 1: Comment
Line 2: Scale factor
Lines 3-5: Lattice vectors (3x3)
Line 6: Element symbols
Line 7: Element counts
Line 8: "Direct" or "Cartesian"
Lines 9+: Coordinates
```

### 1.2 Crystal Structure

**File:** `include/crystal/structure.hpp`
```cpp
namespace defect_gnn::crystal {

class Atom {
public:
    int element;                    // Atomic number
    Eigen::Vector3d position;       // Cartesian coordinates
    Eigen::Vector3d frac_position;  // Fractional coordinates
};

class Structure {
public:
    Structure(const io::VASPStructure& vasp);
    
    const Eigen::Matrix3d& lattice() const;
    const std::vector<Atom>& atoms() const;
    size_t num_atoms() const;
    
    // Get minimum image distance (periodic boundary conditions)
    double distance(size_t i, size_t j) const;
    Eigen::Vector3d displacement(size_t i, size_t j) const;
    
private:
    Eigen::Matrix3d lattice_;
    Eigen::Matrix3d inv_lattice_;  // For PBC
    std::vector<Atom> atoms_;
};

}  // namespace defect_gnn::crystal
```

### 1.3 Neighbor List Construction

**Reference:** `Defect_GNN/utilities.py:34-71` (`structureToGraph`)

**File:** `include/graph/neighbor_list.hpp`
```cpp
namespace defect_gnn::graph {

struct Neighbor {
    size_t idx;
    double distance;
    Eigen::Vector3d displacement;  // Vector from center to neighbor
};

class NeighborList {
public:
    // r_cutoff=10.0 Å, max_neighbors=20 (from paper)
    NeighborList(const crystal::Structure& structure,
                 double r_cutoff = 10.0,
                 size_t max_neighbors = 20);
    
    const std::vector<Neighbor>& neighbors(size_t atom_idx) const;
    
private:
    // Use nanoflann KD-tree for efficient neighbor search
    // Must handle periodic boundary conditions by replicating atoms
    void build_with_pbc(const crystal::Structure& structure);
    
    std::vector<std::vector<Neighbor>> neighbor_lists_;
};

}  // namespace defect_gnn::graph
```

### 1.4 Edge Features (Gaussian RBF)

**Reference:** `Defect_GNN/utilities.py:25-32` (`calculateEdgeAttributes`)

**File:** `include/graph/edge_features.hpp`
```cpp
namespace defect_gnn::graph {

// Gaussian radial basis function expansion
// r_cutoff=10.0, dr=0.1 → 100 bins
// sigma = r_cutoff / 3
//
// Formula: exp(-0.5 * (r_grid - dist)^2 / sigma^2) / sqrt(2*pi) / sigma
//
// Reference: utilities.py:25-32
Eigen::VectorXd gaussian_rbf(double distance,
                              double r_cutoff = 10.0,
                              double dr = 0.1);

}  // namespace defect_gnn::graph
```

### 1.5 Crystal Graph

**File:** `include/graph/crystal_graph.hpp`
```cpp
namespace defect_gnn::graph {

class CrystalGraph {
public:
    CrystalGraph(const crystal::Structure& structure,
                 const NeighborList& neighbors,
                 const std::map<int, Eigen::VectorXd>& atom_embeddings,
                 double r_cutoff = 10.0);
    
    // Node features: [num_atoms, feature_dim]
    // Initially 92-dim CGCNN embeddings, later + topo features
    const Eigen::MatrixXd& node_features() const;
    
    // Edge index: [2, num_edges] - COO format (source, target)
    const Eigen::MatrixXi& edge_index() const;
    
    // Edge attributes: [num_edges, 100] - Gaussian RBF
    const Eigen::MatrixXd& edge_attr() const;
    
    // Target value (formation energy)
    double target() const;
    void set_target(double y);
    
    // Add topological features to node features
    void add_topo_features(const Eigen::MatrixXd& topo);  // [num_atoms, n_pca]
    
    size_t num_nodes() const;
    size_t num_edges() const;
    
private:
    Eigen::MatrixXd node_features_;
    Eigen::MatrixXi edge_index_;
    Eigen::MatrixXd edge_attr_;
    double target_;
};

}  // namespace defect_gnn::graph
```

---

## Phase 2: Persistent Homology (Weeks 3-4)

### 2.1 Strategy: Native C++ Ripser Integration

**Decision:** Use C++ Ripser directly for WebAssembly compatibility and self-contained builds.

**Ripser Source:** https://github.com/Ripser/ripser

### 2.1.1 Ripser Setup

**Step 1:** Clone Ripser into third_party
```bash
cd third_party
git clone https://github.com/Ripser/ripser.git
# Remove .git to avoid nested repo issues
rm -rf ripser/.git
```

**Step 2:** Create wrapper header `include/topology/ripser_wrapper.hpp`
```cpp
#pragma once

#include <Eigen/Dense>
#include <vector>

namespace defect_gnn::topology {

// Wrapper around Ripser's C++ implementation
// Ripser computes persistent homology via the Vietoris-Rips complex

struct PersistencePair {
    double birth;
    double death;
    double persistence() const { return death - birth; }
};

using PersistenceDiagram = std::vector<PersistencePair>;

struct PersistenceResult {
    PersistenceDiagram dim0;  // Connected components (β₀)
    PersistenceDiagram dim1;  // Loops (β₁)
    PersistenceDiagram dim2;  // Voids (β₂)
};

// Compute persistence from distance matrix
// This wraps Ripser's core functionality
// max_dim: highest homology dimension to compute (0, 1, or 2)
// threshold: maximum filtration value (r_cutoff for Betti features)
PersistenceResult compute_persistence_from_distances(
    const Eigen::MatrixXd& distance_matrix,
    int max_dim = 2,
    double threshold = 2.5
);

// Convenience: compute from point cloud (builds distance matrix internally)
PersistenceResult compute_persistence(
    const Eigen::MatrixXd& point_cloud,  // Nx3
    double max_radius = 2.5
);

}  // namespace defect_gnn::topology
```

**Step 3:** Implementation notes for `src/topology/ripser_wrapper.cpp`
```cpp
// Ripser uses a specific input format. Key integration points:
//
// 1. Ripser expects a lower-triangular distance matrix or point cloud
// 2. Main computation is in ripser() function
// 3. Output is persistence pairs per dimension
//
// Integration approach:
// - Include ripser.cpp with modifications for library use
// - Or extract core computation into callable functions
// - Ripser uses compressed_lower_distance_matrix internally
//
// Build flag needed: -DUSE_COEFFICIENTS (for field coefficients)
```

### 2.1.2 CMake Integration

Add to `CMakeLists.txt`:
```cmake
# Ripser (persistent homology)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/ripser/ripser.cpp")
    set(RIPSER_SOURCES "${CMAKE_SOURCE_DIR}/third_party/ripser/ripser.cpp")
    message(STATUS "Using Ripser for persistent homology")
    # Ripser compile options
    add_compile_definitions(USE_COEFFICIENTS)
else()
    message(WARNING "Ripser not found. Run: make deps")
endif()
```

### 2.2 Betti Number Computation

**Reference:** `Defect_GNN/Betti_number.py:93-224`

**Key parameters:**
- `r_cutoff = 2.5` Å (different from graph cutoff!)
- `maxdim = 2` (compute β₀, β₁, β₂)

**File:** `include/topology/persistent_homology.hpp`
```cpp
namespace defect_gnn::topology {

// A persistence pair (birth, death)
struct PersistencePair {
    double birth;
    double death;
    double persistence() const { return death - birth; }
};

// Persistence diagram for one dimension
using PersistenceDiagram = std::vector<PersistencePair>;

// Compute persistence diagrams for a point cloud
// Reference: Betti_number.py:93-141 (get_betti_num)
struct PersistenceResult {
    PersistenceDiagram dim0;  // Connected components
    PersistenceDiagram dim1;  // Loops
    PersistenceDiagram dim2;  // Voids
};

PersistenceResult compute_persistence(
    const Eigen::MatrixXd& point_cloud,  // Nx3
    double max_radius = 2.5
);

}  // namespace defect_gnn::topology
```

### 2.3 Atom-Specific Betti Features

**Reference:** `Defect_GNN/Betti_number.py:143-254`

**Algorithm:**
1. For each atom i in structure:
2. For each element type e:
3. Build point cloud: atom i + all atoms of type e within cutoff
4. Compute persistence diagrams
5. Extract 35 statistical features

**File:** `include/topology/betti_features.hpp`
```cpp
namespace defect_gnn::topology {

// 35 features per atom:
// - β₀: 5 stats (mean, std, max, min, weighted_sum of death times)
// - β₁: 15 stats (5 stats each for birth, death, persistence)
// - β₂: 15 stats (same as β₁)
//
// Reference: Betti_number.py:143-224 (getElementalBettiProperties)

struct BettiStatistics {
    double mean, std, max, min, weighted_sum;
};

// Extract statistics from persistence diagram
BettiStatistics compute_statistics(const PersistenceDiagram& diagram,
                                    bool use_death = true);  // birth or death

// Compute all 35 features for one atom
// Reference: Betti_number.py:226-254 (getElementalBettiFeatures)
Eigen::VectorXd compute_atom_betti_features(
    const crystal::Structure& structure,
    size_t atom_idx,
    double r_cutoff = 2.5
);

// Compute features for all atoms in structure
// Returns [num_atoms, 35] matrix
Eigen::MatrixXd compute_structure_betti_features(
    const crystal::Structure& structure,
    double r_cutoff = 2.5
);

}  // namespace defect_gnn::topology
```

### 2.4 PCA Reduction

**Reference:** `Defect_GNN/dataset_PCA.py:80-81`

**File:** `include/topology/pca.hpp`
```cpp
namespace defect_gnn::topology {

class PCA {
public:
    // Fit PCA on training data
    // n_components: 1, 2, 4, 6, 8, or 10 (paper tests these)
    void fit(const Eigen::MatrixXd& X, int n_components = 6);
    
    // Transform new data
    Eigen::MatrixXd transform(const Eigen::MatrixXd& X) const;
    
    // Save/load fitted PCA
    void save(const std::string& path) const;
    void load(const std::string& path);
    
    // Explained variance ratio
    Eigen::VectorXd explained_variance_ratio() const;
    
private:
    Eigen::MatrixXd components_;    // [n_components, 35]
    Eigen::VectorXd mean_;          // [35]
    Eigen::VectorXd explained_var_;
};

}  // namespace defect_gnn::topology
```

---

## Phase 3: Neural Network Layers (Weeks 5-7)

### 3.1 Design Decision: libtorch vs Custom

**Recommendation:** Start with custom implementation for learning, then optionally port to libtorch for GPU.

### 3.2 Base Tensor Operations

Use Eigen for all matrix operations. Define convenience aliases.

**File:** `include/nn/tensor.hpp`
```cpp
namespace defect_gnn::nn {

using Tensor1D = Eigen::VectorXd;
using Tensor2D = Eigen::MatrixXd;

// Activation functions
Tensor2D relu(const Tensor2D& x);
Tensor2D leaky_relu(const Tensor2D& x, double negative_slope = 0.2);
Tensor2D softmax(const Tensor2D& x, int dim = -1);  // Along rows or cols

// Batch normalization (inference mode)
Tensor2D batch_norm(const Tensor2D& x,
                    const Tensor1D& gamma,
                    const Tensor1D& beta,
                    const Tensor1D& running_mean,
                    const Tensor1D& running_var,
                    double eps = 1e-5);

}  // namespace defect_gnn::nn
```

### 3.3 Linear Layer

**File:** `include/nn/linear.hpp`
```cpp
namespace defect_gnn::nn {

class Linear {
public:
    Linear(int in_features, int out_features, bool bias = true);
    
    // Forward: Y = XW^T + b
    Tensor2D forward(const Tensor2D& x) const;
    
    // Weight access for loading pretrained
    void set_weight(const Tensor2D& W);
    void set_bias(const Tensor1D& b);
    
private:
    Tensor2D weight_;  // [out_features, in_features]
    Tensor1D bias_;    // [out_features]
    bool use_bias_;
};

}  // namespace defect_gnn::nn
```

### 3.4 Message Passing Base

**Reference:** PyTorch Geometric `MessagePassing` class

**File:** `include/nn/conv/message_passing.hpp`
```cpp
namespace defect_gnn::nn {

class MessagePassing {
public:
    // Core message passing:
    // 1. message(x_i, x_j, edge_attr) - compute messages
    // 2. aggregate(messages, edge_index) - sum/mean/max per node
    // 3. update(x_i, aggregated) - update node features
    
    virtual Tensor2D forward(
        const Tensor2D& x,              // [N, F_in]
        const Eigen::MatrixXi& edge_index,  // [2, E]
        const Tensor2D& edge_attr       // [E, D]
    ) = 0;
    
protected:
    // Aggregation: scatter_add, scatter_mean, etc.
    Tensor2D scatter_add(const Tensor2D& src,
                         const Eigen::VectorXi& index,
                         int dim_size);
    
    virtual ~MessagePassing() = default;
};

}  // namespace defect_gnn::nn
```

### 3.5 CGConv (Crystal Graph Convolution)

**Reference:** `Configurational-Disorder/Disorder_GNN/model/model_embedding.py`
**Paper:** Xie & Grossman, Phys. Rev. Lett. 2018

**File:** `include/nn/conv/cgconv.hpp`
```cpp
namespace defect_gnn::nn {

// Crystal Graph Convolution
// x_i' = x_i + Σ_j σ(z_ij^f) ⊙ g(z_ij^s)
// where z_ij = concat(x_i, x_j, e_ij) @ W
//
// Reference: model_embedding.py CGConv usage
class CGConv : public MessagePassing {
public:
    CGConv(int channels, int edge_dim, bool batch_norm = true);
    
    Tensor2D forward(const Tensor2D& x,
                     const Eigen::MatrixXi& edge_index,
                     const Tensor2D& edge_attr) override;
    
    void load_weights(/* weight dict */);
    
private:
    Linear lin_f_;  // Filter network
    Linear lin_s_;  // Core network
    // BatchNorm params if used
};

}  // namespace defect_gnn::nn
```

### 3.6 GATv2Conv (Graph Attention v2)

**Reference:** `model_embedding.py` uses `GATv2Conv`
**Paper:** Brody et al., "How Attentive are Graph Attention Networks?"

**File:** `include/nn/conv/gatv2conv.hpp`
```cpp
namespace defect_gnn::nn {

// GATv2: fixes static attention problem
// α_ij = softmax_j(a^T LeakyReLU(W[x_i || x_j || e_ij]))
// x_i' = Σ_j α_ij W_t x_j
//
// Multi-head: concatenate or average heads
class GATv2Conv : public MessagePassing {
public:
    GATv2Conv(int in_channels, int out_channels,
              int heads = 1, int edge_dim = -1,
              double negative_slope = 0.2,
              double dropout = 0.0);  // Dropout only in training
    
    Tensor2D forward(const Tensor2D& x,
                     const Eigen::MatrixXi& edge_index,
                     const Tensor2D& edge_attr) override;
    
private:
    int heads_;
    Linear lin_l_, lin_r_;  // Left and right projections
    Linear lin_edge_;       // Edge feature projection
    Tensor1D att_;          // Attention vector
    double negative_slope_;
};

}  // namespace defect_gnn::nn
```

### 3.7 TransformerConv

**Reference:** `model_embedding.py` uses `TransformerConv`
**Paper:** Shi et al., "Masked Label Prediction"

**File:** `include/nn/conv/transformer_conv.hpp`
```cpp
namespace defect_gnn::nn {

// Graph Transformer
// α_ij = softmax_j((W_Q x_i)^T (W_K x_j + W_E e_ij) / sqrt(d))
// x_i' = W_1 x_i + Σ_j α_ij (W_V x_j + W_E e_ij)
class TransformerConv : public MessagePassing {
public:
    TransformerConv(int in_channels, int out_channels,
                    int heads = 1, int edge_dim = -1);
    
    Tensor2D forward(const Tensor2D& x,
                     const Eigen::MatrixXi& edge_index,
                     const Tensor2D& edge_attr) override;
    
private:
    int heads_;
    Linear lin_query_, lin_key_, lin_value_;
    Linear lin_edge_;
    Linear lin_skip_;  // Skip connection W_1
};

}  // namespace defect_gnn::nn
```

### 3.8 Global Pooling

**CRITICAL:** Paper shows global_max_pool is key for defect tasks!

**Reference:** Paper Section "Model Performance" + Table 2

**File:** `include/nn/pool/global_pool.hpp`
```cpp
namespace defect_gnn::nn {

// Global max pooling: take max over all nodes per graph
// batch: [N] tensor indicating which graph each node belongs to
// Returns: [num_graphs, F]
//
// IMPORTANT: Paper shows this outperforms mean pooling for defects
Tensor2D global_max_pool(const Tensor2D& x,
                          const Eigen::VectorXi& batch,
                          int num_graphs);

// Also implement mean for comparison
Tensor2D global_mean_pool(const Tensor2D& x,
                           const Eigen::VectorXi& batch,
                           int num_graphs);

}  // namespace defect_gnn::nn
```

### 3.9 Full Model

**Reference:** `model_embedding.py` - all three model classes

**File:** `include/nn/model.hpp`
```cpp
namespace defect_gnn::nn {

enum class ModelType { CGNN, GAT, Transformer };
enum class PoolType { Max, Mean };

class DefectGNN {
public:
    DefectGNN(ModelType type,
              int num_hidden_layers,
              int num_hidden_channels,
              int num_edge_features,
              int num_topo_features,  // PCA components (0 to disable)
              int num_heads = 1,      // For GAT/Transformer
              PoolType pool = PoolType::Max);
    
    // Forward pass
    // Returns: [batch_size] formation energy predictions
    Tensor1D forward(const std::vector<graph::CrystalGraph>& batch);
    
    // Load pretrained weights from PyTorch checkpoint
    void load_weights(const std::string& path);
    
private:
    // Embedding: maps atomic number to hidden_channels - topo_features
    Tensor2D embedding_;  // [100, embed_dim]
    
    // Convolution layers
    std::vector<std::unique_ptr<MessagePassing>> conv_layers_;
    
    // MLP head
    Linear lin1_, lin2_;
    // BatchNorm after lin1
    
    int num_topo_features_;
    PoolType pool_type_;
};

}  // namespace defect_gnn::nn
```

**Key difference from Configurational-Disorder code:**
```cpp
// In forward():
// 1. Embed atomic numbers
x = embedding_lookup(data.x);  // [N, hidden - topo_dim]

// 2. Concatenate topo features (THE KEY MODIFICATION)
if (num_topo_features_ > 0) {
    x = concat(x, data.topo_features, dim=1);  // [N, hidden]
}

// 3. Message passing with SKIP CONNECTIONS (sum features from all layers)
global_feature = zeros(batch_size, hidden);
for (auto& conv : conv_layers_) {
    x = relu(conv->forward(x, edge_index, edge_attr));
    global_feature += global_max_pool(x, batch);  // MAX not mean!
}

// 4. MLP
return lin2(relu(lin1(global_feature)));
```

---

## Phase 4: Training Infrastructure (Weeks 8-9)

### 4.1 Dataset Class

**Reference:** `Defect_GNN/dataset_PCA.py` + `Configurational-Disorder/Disorder_GNN/dataset.py`

**File:** `include/train/dataset.hpp`
```cpp
namespace defect_gnn::train {

class DefectDataset {
public:
    DefectDataset(const std::string& root,
                  int n_pca_components = 6,
                  bool precompute_betti = true);
    
    // Access
    const graph::CrystalGraph& operator[](size_t idx) const;
    size_t size() const;
    
    // Shuffle for training
    void shuffle();
    
    // Split into train/val/test
    static std::tuple<DefectDataset, DefectDataset, DefectDataset>
    random_split(DefectDataset& full,
                 double train_ratio = 0.6,
                 double val_ratio = 0.2);
    
private:
    std::vector<graph::CrystalGraph> graphs_;
    topology::PCA pca_;  // Fitted on training set
};

}  // namespace defect_gnn::train
```

### 4.2 DataLoader (Batching)

**File:** `include/train/dataloader.hpp`
```cpp
namespace defect_gnn::train {

// Collate multiple graphs into a batch
// - Concatenate node features with batch index
// - Offset edge indices
// - Stack targets
struct BatchedGraph {
    Eigen::MatrixXd x;           // [total_nodes, F]
    Eigen::MatrixXi edge_index;  // [2, total_edges]
    Eigen::MatrixXd edge_attr;   // [total_edges, D]
    Eigen::MatrixXd topo;        // [total_nodes, n_pca]
    Eigen::VectorXi batch;       // [total_nodes] - graph membership
    Eigen::VectorXd y;           // [batch_size] - targets
    int num_graphs;
};

class DataLoader {
public:
    DataLoader(DefectDataset& dataset, int batch_size, bool shuffle = true);
    
    // Iterator interface
    class Iterator;
    Iterator begin();
    Iterator end();
    
    size_t num_batches() const;
    
private:
    DefectDataset& dataset_;
    int batch_size_;
    std::vector<size_t> indices_;
};

}  // namespace defect_gnn::train
```

### 4.3 Optimizer (Adam)

**File:** `include/train/optimizer.hpp`
```cpp
namespace defect_gnn::train {

class Adam {
public:
    Adam(/* model parameters */, double lr = 1e-3,
         double beta1 = 0.9, double beta2 = 0.999,
         double weight_decay = 5e-4, double eps = 1e-8);
    
    void step(/* gradients */);
    void zero_grad();
    
private:
    // First and second moment estimates for each parameter
};

}  // namespace defect_gnn::train
```

**Note:** For inference-only (forward pass), you don't need backprop. Training in C++ is educational but not required if you train in Python and export weights.

### 4.4 Trainer

**Reference:** `Configurational-Disorder/Disorder_GNN/GNN.py`

**File:** `include/train/trainer.hpp`
```cpp
namespace defect_gnn::train {

struct TrainConfig {
    int max_epochs = 200;
    int batch_size = 128;
    double lr = 0.01;
    double weight_decay = 5e-4;
    bool early_stopping = true;
    int patience = 8;
};

class Trainer {
public:
    Trainer(nn::DefectGNN& model, const TrainConfig& config);
    
    void fit(DefectDataset& train_set, DefectDataset& val_set);
    double evaluate(DefectDataset& test_set);  // Returns MAE
    
    // Loss: MAE (paper uses sum of abs errors)
    double compute_loss(const Tensor1D& pred, const Tensor1D& target);
    
private:
    nn::DefectGNN& model_;
    TrainConfig config_;
    std::vector<double> train_losses_, val_losses_;
};

}  // namespace defect_gnn::train
```

---

## Phase 5: Validation & Testing (Week 10)

### 5.1 Test Strategy

1. **Unit tests** for each component
2. **Integration test**: Compare forward pass output with PyTorch
3. **Reproduce paper results**: Train from scratch, verify MAE

### 5.2 Validation Script

**File:** `scripts/validate_against_pytorch.py`
```python
"""
Load same weights in both PyTorch and C++, 
run forward pass on same input,
compare outputs.
"""
# 1. Export PyTorch model weights to JSON/binary
# 2. Run C++ forward pass
# 3. Compare numerically (should match to ~1e-5)
```

### 5.3 Target Metrics (from paper)

| Model | Pooling | Topo Features | MAE (eV) |
|-------|---------|---------------|----------|
| Transformer | Mean | No | 1.55 |
| Transformer | Max | No | 1.55 |
| Transformer | Max | Yes | **0.72** |
| GAT | Max | Yes | 0.94 |
| CGNN | Max | Yes | 0.98 |

**Success criterion:** Achieve MAE ≤ 0.80 eV with Transformer + Max Pool + Topo Features.

---

## Phase 6: WebAssembly Visualization (Backlog)

### 6.1 Scope

Interactive 3D crystal viewer where users can:
1. Load perovskite structure
2. Click atoms to create vacancies
3. See predicted formation energy update in real-time
4. View "heatmap" of formation energies for all sites

### 6.2 Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Browser                               │
│  ┌─────────────────┐    ┌─────────────────────────────────┐ │
│  │   React UI      │◄──►│   WebAssembly Module            │ │
│  │   (Three.js)    │    │   - Structure parsing           │ │
│  │                 │    │   - Graph construction          │ │
│  │   - 3D viewer   │    │   - Betti features (precomputed)│ │
│  │   - Controls    │    │   - GNN forward pass            │ │
│  │   - Heatmap     │    │                                 │ │
│  └────────┬────────┘    └──────────────────────────────────┘│
│           │                                                  │
│           └──── Embind API ─────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

### 6.3 WASM API (Embind)

```cpp
// Exposed to JavaScript
EMSCRIPTEN_BINDINGS(defect_gnn) {
    function("loadStructure", &load_structure_from_string);
    function("createVacancy", &create_vacancy);
    function("predictFormationEnergy", &predict_formation_energy);
    function("getAllSiteEnergies", &get_all_site_energies);  // For heatmap
    
    class_<Structure>("Structure")
        .function("numAtoms", &Structure::num_atoms)
        .function("getAtomPosition", &Structure::get_atom_position)
        .function("getAtomElement", &Structure::get_atom_element);
}
```

### 6.4 Simplifications for WASM

1. **Pre-compute Betti features** for sample structures (avoid Ripser in browser)
2. **Quantize weights** to reduce model size
3. **Limit to Transformer** (best performing)
4. **Use ONNX Runtime Web** as alternative to custom inference

### 6.5 Directory Addition

```
web/
├── public/
│   └── wasm/
│       ├── defect_gnn.js       # Emscripten glue
│       └── defect_gnn.wasm     # Binary
├── src/
│   ├── components/
│   │   ├── CrystalViewer.tsx   # Three.js 3D view
│   │   ├── DefectSelector.tsx  # Click-to-create-vacancy
│   │   ├── EnergyDisplay.tsx   # Show prediction
│   │   └── HeatmapOverlay.tsx  # Color atoms by energy
│   ├── hooks/
│   │   └── useDefectGNN.ts     # WASM loading & API
│   └── App.tsx
└── sample_structures/
    └── BaTiO3_supercell.vasp   # Pre-loaded demo
```

---

## Appendix A: Key Reference Mapping

| Your File | Reference Source | Notes |
|-----------|------------------|-------|
| `vasp_parser.cpp` | `Betti_number.py:15-46` | VASP format parsing |
| `edge_features.cpp` | `utilities.py:25-32` | Gaussian RBF |
| `crystal_graph.cpp` | `utilities.py:34-71` | `structureToGraph` |
| `betti_features.cpp` | `Betti_number.py:93-254` | All Betti computation |
| `cgconv.cpp` | PyG `CGConv` + paper | |
| `gatv2conv.cpp` | PyG `GATv2Conv` | |
| `transformer_conv.cpp` | PyG `TransformerConv` | |
| `model.cpp` | `model_embedding.py` | **Add topo concat + max pool** |
| `dataset.cpp` | `dataset_PCA.py` | |
| `trainer.cpp` | `GNN.py` | Training loop |

---

## Appendix B: Third-Party Libraries

| Library | Purpose | Install |
|---------|---------|---------|
| **Eigen** | Linear algebra | Header-only, copy to `third_party/` |
| **nlohmann/json** | JSON parsing | Header-only |
| **nanoflann** | KD-tree | Header-only |
| **Ripser** | Persistent homology | Clone from GitHub |
| **Emscripten** | WASM compilation | `emsdk install latest` |

---

## Appendix C: Milestones Checklist

### Phase 1: Data Pipeline
- [ ] Parse VASP files
- [ ] Build crystal structures with PBC
- [ ] Construct neighbor lists
- [ ] Generate Gaussian RBF edge features
- [ ] Build crystal graph objects

### Phase 2: Persistent Homology
- [ ] Compute persistence diagrams (via Ripser or Python)
- [ ] Extract 35 Betti statistics per atom
- [ ] Implement PCA for dimensionality reduction
- [ ] Integrate topo features into graph

### Phase 3: Neural Network
- [ ] Implement Linear layer
- [ ] Implement CGConv
- [ ] Implement GATv2Conv
- [ ] Implement TransformerConv
- [ ] Implement global_max_pool
- [ ] Build full model with topo feature injection

### Phase 4: Training
- [ ] Dataset class with preprocessing
- [ ] DataLoader with batching
- [ ] Training loop (or just use PyTorch + export)
- [ ] Weight loading from PyTorch checkpoint

### Phase 5: Validation
- [ ] Unit tests pass
- [ ] Forward pass matches PyTorch
- [ ] Reproduce paper MAE (~0.72 eV)

### Phase 6: WebAssembly (Backlog)
- [ ] Compile core to WASM
- [ ] Embind API
- [ ] Three.js crystal viewer
- [ ] Interactive defect creation
- [ ] Formation energy heatmap
