# Defect GNN

C++ implementation of graph neural networks for predicting vacancy formation energies in crystalline materials, compiled to WebAssembly for interactive visualization.

**[Live Demo](https://defectgnn.bamarler.com)** · **[Paper (Fang & Yan, 2025)](https://doi.org/10.1021/acs.chemmater.4c03028)**

---

## Overview

Implements Fang & Yan's methodology: crystal structures → graph representations where atoms are nodes (with learned element embeddings) and edges encode interatomic distances via Gaussian radial basis functions. The model combines graph convolutional layers with persistent homology descriptors (Betti numbers capturing topological voids) to predict formation energies.

**Visualization modes:**

- **Structure**: Unit cell atomic positions from VASP POSCAR files
- **Graph**: Neighbor connectivity computed via KD-tree search with periodic boundary conditions (minimum image convention)

## Applications

- **Semiconductors** — dopant activation, carrier lifetime, device performance
- **Batteries & Catalysts** — ion mobility, active site stability, degradation pathways

## Status

| Component                                               | Status         |
| ------------------------------------------------------- | -------------- |
| VASP → Structure parsing                                | ✅ Complete    |
| Crystal graph construction (KD-tree, PBC, Gaussian RBF) | ✅ Complete    |
| Initial structure/graph visualization                   | ✅ Complete    |
| Persistent homology features (Ripser)                   | 🚧 In progress |
| GNN training pipeline                                   | 🚧 In progress |
| Persistent homology visualization                       | 📋 Planned     |
| GNN performance visualization                           | 📋 Planned     |

## Tech Stack

**Core**: C++17, Eigen, nanoflann · **Web**: Emscripten, Next.js, 3Dmol.js · **Deploy**: Vercel

## License

MIT
