# Phase 1: Data Pipeline Implementation Guide

## Table of Contents

1. [Mathematical Background](#1-mathematical-background)
   - [Periodic Boundary Conditions (PBC)](#11-periodic-boundary-conditions-pbc)
   - [Minimum Image Convention](#12-minimum-image-convention)
   - [KD-Trees](#13-kd-trees)
   - [Gaussian Radial Basis Functions](#14-gaussian-radial-basis-functions)
2. [Data Flow Overview](#2-data-flow-overview)
3. [Implementation Details](#3-implementation-details)
   - [VASP Parser](#31-vasp-parser)
   - [Crystal Structure](#32-crystal-structure)
   - [Neighbor List](#33-neighbor-list)
   - [Edge Features](#34-edge-features)
   - [Crystal Graph](#35-crystal-graph)

---

## 1. Mathematical Background

### 1.1 Periodic Boundary Conditions (PBC)

**The Problem**: Crystals are infinite repeating patterns, but we can only store a finite "unit cell."

**The Solution**: Treat the unit cell as tiling infinitely in all directions.

```
Real crystal structure (infinite):
┌─────┬─────┬─────┬─────┬─────┐
│  A  │  A  │  A  │  A  │  A  │
│   B │   B │   B │   B │   B │
├─────┼─────┼─────┼─────┼─────┤
│  A  │  A  │  A  │  A  │  A  │
│   B │   B │   B │   B │   B │
└─────┴─────┴─────┴─────┴─────┘

What we store (single unit cell):
┌─────┐
│  A  │   A at fractional (0.1, 0.5)
│   B │   B at fractional (0.9, 0.5)
└─────┘
```

**Lattice Vectors**: The unit cell is defined by three vectors **a**, **b**, **c** forming a 3×3 matrix **L**:

```
L = [ a₁  a₂  a₃ ]    (row 0 = a vector)
    [ b₁  b₂  b₃ ]    (row 1 = b vector)
    [ c₁  c₂  c₃ ]    (row 2 = c vector)
```

**Coordinate Systems**:

- **Fractional coordinates** `f = (f₁, f₂, f₃)`: Position as fraction of lattice vectors (0 to 1)
- **Cartesian coordinates** `r`: Position in Ångströms

**Conversion formulas**:

```
Fractional → Cartesian:  r = f · L  (or r = Lᵀ · f depending on convention)
Cartesian → Fractional:  f = r · L⁻¹
```

In Eigen (with row-major lattice):

```cpp
// frac_coords is 1x3, lattice is 3x3 (rows are a, b, c)
Eigen::Vector3d cart = frac_coords.transpose() * lattice;  // or lattice.transpose() * frac
```

---

### 1.2 Minimum Image Convention

**Problem**: Given atoms A and B in the unit cell, what's their "true" distance considering periodicity?

**Naive distance** (ignoring PBC):

```
A at (0.1, 0.5, 0.5)  →  in 10Å cell: (1, 5, 5) Å
B at (0.9, 0.5, 0.5)  →  in 10Å cell: (9, 5, 5) Å

Direct distance = 8 Å  ← WRONG!
```

**Minimum image distance**:

```
B's periodic image at (-0.1, 0.5, 0.5) → (-1, 5, 5) Å

Distance to image = 2 Å  ← CORRECT!
```

**Algorithm**:

1. Compute displacement vector in fractional coordinates: `Δf = f_B - f_A`
2. Wrap each component to [-0.5, 0.5): `Δf_i = Δf_i - round(Δf_i)`
3. Convert to Cartesian: `Δr = Δf · L`
4. Distance = `|Δr|`

```
Visual (1D, cell length = 10):

     A         B           Direct: 8
     │─────────│
     ├─────────┤
 ────┴─────────┴─────────────────────
     0    1    ↑         9    10
               │
            Image of B at position -1

     A     B'                  Via image: 2 ✓
     │     │
     ├─────┤
 ────┴─────┴─────────────────────
    -1     0    1
```

**Pseudocode**:

```
MINIMUM-IMAGE-DISPLACEMENT(frac_i, frac_j, L)
─────────────────────────────────────────────
Input:  frac_i, frac_j ∈ ℝ³  (fractional coordinates)
        L ∈ ℝ³ˣ³            (lattice matrix, rows = a, b, c)
Output: Δr ∈ ℝ³             (Cartesian displacement vector)

1.  Δf ← frac_j − frac_i

2.  for k ← 0 to 2 do                    ▷ Wrap to [−0.5, 0.5)
3.      Δf[k] ← Δf[k] − round(Δf[k])

4.  Δr ← Lᵀ · Δf                         ▷ Convert to Cartesian

5.  return Δr
```

---

### 1.3 KD-Trees

**Problem**: Finding all neighbors within radius `r` for N atoms is O(N²) naively.

**Solution**: A KD-tree is a binary space-partitioning structure that enables O(log N) average-case neighbor queries.

**How it works**:

1. **Build** (O(N log N)): Recursively split points by median along alternating axes
2. **Query** (O(log N) average): Prune branches that can't contain neighbors

```
2D Example: Points A(2,3), B(5,4), C(9,1), D(4,7), E(8,8)

                    D(4,7)          ← Split on X at x=4
                   /      \
             A(2,3)        E(8,8)   ← Split on Y
              |           /     \
            [leaf]    C(9,1)   [leaf]
```

**Range query for point Q within radius r**:

```
RANGE-SEARCH(node, Q, r, results)
────────────────────────────────
Input:  node        (current KD-tree node)
        Q ∈ ℝ³      (query point)
        r ∈ ℝ       (search radius)
        results     (accumulator list)

1.  if node = NIL then return

2.  d ← ‖node.point − Q‖
3.  if d ≤ r then
4.      results.append(node.point)

5.  axis ← node.split_axis
6.  δ ← Q[axis] − node.point[axis]

7.  if δ < 0 then                        ▷ Determine near/far children
8.      near ← node.left
9.      far ← node.right
10. else
11.     near ← node.right
12.     far ← node.left

13. RANGE-SEARCH(near, Q, r, results)    ▷ Always search near side

14. if |δ| ≤ r then                      ▷ Only search far if sphere crosses plane
15.     RANGE-SEARCH(far, Q, r, results)
```

**For PBC**: We can't directly use minimum image in a KD-tree because it breaks the spatial partitioning assumption. Instead:

**Image Expansion Strategy**:

1. Determine how many periodic images needed: `n = ceil(r_cutoff / min_lattice_length)`
2. Create expanded point cloud with all images
3. Build KD-tree on expanded cloud
4. Query returns original atom indices (stored with each image point)

```
Original cell:        Expanded for r_cutoff (1 image each direction):
┌─────┐               ┌─────┬─────┬─────┐
│ A B │       →       │ A B │ A B │ A B │
│ C   │               │ C   │ C   │ C   │
└─────┘               ├─────┼─────┼─────┤
                      │ A B │ A B │ A B │  ← Original cell
                      │ C   │ C   │ C   │
                      ├─────┼─────┼─────┤
                      │ A B │ A B │ A B │
                      │ C   │ C   │ C   │
                      └─────┴─────┴─────┘
```

**nanoflann** (header-only C++ KD-tree library):

The adaptor requires these interface functions:

```
PointCloud Adaptor Interface
────────────────────────────
KDTREE-GET-POINT-COUNT()  →  number of points
KDTREE-GET-PT(idx, dim)   →  coordinate value at index, dimension
KDTREE-GET-BBOX(bbox)     →  optional bounding box (return false to skip)

Data stored:
  points[]           ∈ ℝ³       (Cartesian positions of all image points)
  original_indices[] ∈ ℤ        (which atom each image came from)
```

---

### 1.4 Gaussian Radial Basis Functions

**Problem**: A single distance value (scalar) isn't a good feature for neural networks.

**Solution**: Expand it into a rich vector using Gaussian RBF encoding.

**Formula**:

```
RBF(d) = [g₀, g₁, ..., g_{n-1}]

where gₖ = (1 / σ√(2π)) · exp(-0.5 · (rₖ - d)² / σ²)

Parameters:
  - r_cutoff = 10.0 Å
  - dr = 0.1 Å
  - n_bins = r_cutoff / dr = 100
  - rₖ = k · dr  (centers at 0, 0.1, 0.2, ..., 9.9)
  - σ = r_cutoff / 3 ≈ 3.33 Å
```

**Intuition**: Each bin detects "how close is this distance to my center?"

```
Distance d = 3.5 Å

     ▲ gₖ value
     │
 0.12├─────────────────╭──╮───────────────────
     │                ╱    ╲
 0.08├───────────────╱──────╲──────────────────
     │              ╱        ╲
 0.04├─────────────╱──────────╲────────────────
     │            ╱            ╲
 0.00├───────────────────────────────────────→ rₖ
     0         3.5              10 Å
               ↑
            Peak at d
```

**Pseudocode**:

```
GAUSSIAN-RBF(d, r_cutoff, Δr)
─────────────────────────────
Input:  d ∈ ℝ         (distance in Å)
        r_cutoff ∈ ℝ  (maximum radius, default 10.0)
        Δr ∈ ℝ        (bin spacing, default 0.1)
Output: g ∈ ℝⁿ        (RBF feature vector)

1.  n ← ⌊r_cutoff / Δr⌋                  ▷ Number of bins (100)
2.  σ ← r_cutoff / 3                     ▷ Gaussian width (3.33)
3.  norm ← 1 / (σ · √(2π))

4.  for k ← 0 to n−1 do
5.      center ← k · Δr                  ▷ Bin centers: 0, 0.1, 0.2, ...
6.      g[k] ← norm · exp(−0.5 · (center − d)² / σ²)

7.  return g
```

---

## 2. Data Flow Overview

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                          PHASE 1: DATA PIPELINE                              │
└──────────────────────────────────────────────────────────────────────────────┘

                                 RAW INPUTS
                                     │
          ┌──────────────────────────┼──────────────────────────┐
          ▼                          ▼                          ▼
    ┌───────────┐           ┌─────────────────┐         ┌─────────────┐
    │ POSCAR    │           │ atomic_embedding│         │  data.csv   │
    │ (VASP)    │           │ _CGCNN.json     │         │  (labels)   │
    └─────┬─────┘           └────────┬────────┘         └──────┬──────┘
          │                          │                         │
          ▼                          ▼                         ▼
  ┌───────────────┐         ┌────────────────┐         ┌──────────────┐
  │ parse_vasp()  │         │parse_atom_     │         │parse_defect_ │
  │               │         │embeddings()    │         │csv()         │
  └───────┬───────┘         └───────┬────────┘         └──────┬───────┘
          │                         │                         │
          ▼                         ▼                         │
  ┌───────────────┐         ┌────────────────┐                │
  │ VASPStructure │         │map<int,        │                │
  │ - lattice     │         │  VectorXd>     │                │
  │ - frac_coords │         │ (Z → 92-dim)   │                │
  │ - elements    │         └───────┬────────┘                │
  └───────┬───────┘                 │                         │
          │                         │                         │
          ▼                         │                         │
  ┌───────────────┐                 │                         │
  │ Structure()   │                 │                         │
  │ constructor   │                 │                         │
  └───────┬───────┘                 │                         │
          │                         │                         │
          ▼                         │                         │
  ┌───────────────┐                 │                         │
  │ crystal::     │                 │                         │
  │ Structure     │                 │                         │
  │ - lattice_    │                 │                         │
  │ - inv_lattice_│                 │                         │
  │ - atoms_      │                 │                         │
  └───────┬───────┘                 │                         │
          │                         │                         │
          ▼                         │                         │
  ┌───────────────┐                 │                         │
  │ NeighborList  │                 │                         │
  │ constructor   │                 │                         │
  │ - builds PBC  │                 │                         │
  │   images      │                 │                         │
  │ - KD-tree     │                 │                         │
  │ - queries     │                 │                         │
  └───────┬───────┘                 │                         │
          │                         │                         │
          ▼                         │                         │
  ┌───────────────────┐             │                         │
  │ neighbor_lists_   │             │                         │
  │ [i] = [(j, d,Δr)]│             │                         │
  │  for each atom i  │             │                         │
  └───────┬───────────┘             │                         │
          │                         │                         │
          │                         │                         │
          └────────────┬────────────┘                         │
                       │                                      │
                       ▼                                      │
               ┌───────────────┐                              │
               │ CrystalGraph  │                              │
               │ constructor   │                              │
               └───────┬───────┘                              │
                       │                                      │
                       ▼                                      │
               ┌───────────────────────────────────┐          │
               │         CrystalGraph              │          │
               │  ┌─────────────────────────────┐  │          │
               │  │ node_features_ [N × 92]     │  │          │
               │  │  (from atom_embeddings)     │  │          │
               │  └─────────────────────────────┘  │          │
               │  ┌─────────────────────────────┐  │          │
               │  │ edge_index_ [2 × E]         │  │          │
               │  │  [[src₀,src₁,...],          │  │          │
               │  │   [dst₀,dst₁,...]]          │  │          │
               │  └─────────────────────────────┘  │          │
               │  ┌─────────────────────────────┐  │          │
               │  │ edge_attr_ [E × 100]        │  │          │
               │  │  (Gaussian RBF of distance) │  │          │
               │  └─────────────────────────────┘  │          │
               │  ┌─────────────────────────────┐  │◄─────────┘
               │  │ target_ (formation energy)  │  │
               │  └─────────────────────────────┘  │
               └───────────────────────────────────┘
                              │
                              ▼
                      Ready for GNN!
```

---

## 3. Implementation Details

### 3.1 VASP Parser

**File**: `src/io/vasp_parser.cpp`

**VASP POSCAR format**:

```
Comment line (ignored)
1.0                          ← Scale factor
   10.0    0.0    0.0        ← Lattice vector a
    0.0   10.0    0.0        ← Lattice vector b
    0.0    0.0   10.0        ← Lattice vector c
Sr Ti O                      ← Element symbols
1 1 3                        ← Counts per element
Direct                       ← Coordinate type
0.0 0.0 0.0                  ← Fractional coords
0.5 0.5 0.5
0.5 0.5 0.0
0.5 0.0 0.5
0.0 0.5 0.5
```

**Pseudocode**:

```
PARSE-VASP(filepath)
────────────────────
Input:  filepath (path to POSCAR file)
Output: V (VASPStructure with lattice, elements, counts, frac_coords, atom_types)

1.  file ← OPEN(filepath)

2.  comment ← READ-LINE(file)            ▷ Line 1: skip

3.  scale ← READ-FLOAT(file)             ▷ Line 2: scale factor

4.  L ← 3×3 zero matrix                  ▷ Lines 3-5: lattice vectors
5.  for i ← 0 to 2 do
6.      for j ← 0 to 2 do
7.          L[i,j] ← READ-FLOAT(file)
8.  L ← L · scale

9.  line ← READ-LINE(file)               ▷ Line 6: element symbols
10. elements ← SPLIT-WHITESPACE(line)

11. line ← READ-LINE(file)               ▷ Line 7: counts per element
12. counts ← PARSE-INTEGERS(line)
13. N ← SUM(counts)                      ▷ Total atom count

14. line ← READ-LINE(file)               ▷ Line 8: coordinate type
15. is_direct ← (line[0] = 'D' or line[0] = 'd')

16. F ← N×3 matrix                       ▷ Lines 9+: coordinates
17. for i ← 0 to N−1 do
18.     F[i,0], F[i,1], F[i,2] ← READ-FLOATS(file, 3)

19. if not is_direct then                ▷ Convert Cartesian → fractional
20.     F ← F · L⁻¹

21. atom_types ← empty list              ▷ Build per-atom element index
22. for elem_idx ← 0 to |elements|−1 do
23.     for j ← 0 to counts[elem_idx]−1 do
24.         atom_types.append(elem_idx)

25. V.lattice ← L
26. V.elements ← elements
27. V.counts ← counts
28. V.frac_coords ← F
29. V.atom_types ← atom_types

30. return V
```

**Element symbol → Atomic number** (needed for embeddings):

```cpp
// Store in a header or use a lookup table
const std::map<std::string, int> ELEMENT_TO_Z = {
    {"H", 1}, {"He", 2}, {"Li", 3}, {"Be", 4}, {"B", 5},
    {"C", 6}, {"N", 7}, {"O", 8}, {"F", 9}, {"Ne", 10},
    // ... through element 92
    {"Sr", 38}, {"Ti", 22}, {"Ba", 56}, {"Pb", 82}, ...
};
```

---

### 3.2 Crystal Structure

**File**: `src/crystal/structure.cpp`

**Key responsibility**: Store lattice + atoms, provide minimum-image distance queries.

```
STRUCTURE-INIT(V)
─────────────────
Input:  V (VASPStructure from parser)
Output: S (Structure with lattice, inv_lattice, atoms)

1.  S.lattice ← V.lattice
2.  S.inv_lattice ← INVERSE(V.lattice)

3.  for i ← 0 to ROWS(V.frac_coords)−1 do
4.      a.element ← ELEMENT-TO-Z[V.elements[V.atom_types[i]]]
5.      a.frac_position ← V.frac_coords[i]
6.      a.position ← S.latticeᵀ · a.frac_position    ▷ Frac → Cartesian
7.      S.atoms.append(a)

8.  return S


STRUCTURE-DISTANCE(S, i, j)
───────────────────────────
Input:  S (Structure), i, j (atom indices)
Output: d ∈ ℝ (minimum image distance)

1.  Δr ← STRUCTURE-DISPLACEMENT(S, i, j)
2.  return ‖Δr‖


STRUCTURE-DISPLACEMENT(S, i, j)
───────────────────────────────
Input:  S (Structure), i, j (atom indices)
Output: Δr ∈ ℝ³ (minimum image displacement vector)

1.  Δf ← S.atoms[j].frac_position − S.atoms[i].frac_position

2.  for k ← 0 to 2 do                    ▷ Wrap to [−0.5, 0.5)
3.      Δf[k] ← Δf[k] − round(Δf[k])

4.  Δr ← S.latticeᵀ · Δf

5.  return Δr
```

---

### 3.3 Neighbor List

**File**: `src/graph/neighbor_list.cpp`

This is the most complex component. We use **image expansion + KD-tree**.

**Step 1**: Determine replication bounds

```
COMPUTE-NUM-IMAGES(L, r_cutoff)
───────────────────────────────
Input:  L ∈ ℝ³ˣ³    (lattice matrix)
        r_cutoff ∈ ℝ (neighbor search radius)
Output: n ∈ ℤ       (number of image replications per direction)

1.  ℓ_min ← min(‖L[0]‖, ‖L[1]‖, ‖L[2]‖)   ▷ Shortest lattice vector
2.  n ← ⌈r_cutoff / ℓ_min⌉ + 1
3.  return n
```

**Step 2**: Create expanded point cloud

```
CREATE-IMAGE-CLOUD(S, n)
────────────────────────
Input:  S (Structure with N atoms)
        n (number of image replications per direction)
Output: cloud (list of {position, original_index} pairs)

1.  cloud ← empty list
2.  L ← S.lattice

3.  for nₐ ← −n to +n do
4.      for nᵦ ← −n to +n do
5.          for nᵧ ← −n to +n do
6.              offset ← nₐ·L[0] + nᵦ·L[1] + nᵧ·L[2]    ▷ Image translation
7.
8.              for i ← 0 to N−1 do
9.                  pt.position ← S.atoms[i].position + offset
10.                 pt.original_index ← i
11.                 cloud.append(pt)

12. return cloud
```

**Step 3**: Build KD-tree and query

```
BUILD-NEIGHBOR-LIST-WITH-PBC(S, r_cutoff, max_neighbors)
────────────────────────────────────────────────────────
Input:  S (Structure with N atoms)
        r_cutoff ∈ ℝ (search radius)
        max_neighbors ∈ ℤ (maximum neighbors per atom)
Output: neighbor_lists (array of neighbor lists for each atom)

1.  n ← COMPUTE-NUM-IMAGES(S.lattice, r_cutoff)
2.  cloud ← CREATE-IMAGE-CLOUD(S, n)

3.  tree ← BUILD-KD-TREE(cloud)          ▷ Using nanoflann

4.  neighbor_lists ← array of N empty lists

5.  for i ← 0 to N−1 do
6.      Q ← S.atoms[i].position          ▷ Query point
7.      matches ← RADIUS-SEARCH(tree, Q, r_cutoff)
8.
9.      neighbors ← empty list
10.     for each (cloud_idx, dist) in matches do
11.         orig_idx ← cloud[cloud_idx].original_index
12.
13.         if orig_idx = i and dist < ε then    ▷ Skip self
14.             continue
15.
16.         Δr ← cloud[cloud_idx].position − Q
17.         neighbors.append({idx: orig_idx, distance: dist, displacement: Δr})
18.
19.     SORT neighbors by distance (ascending)
20.
21.     if |neighbors| > max_neighbors then
22.         neighbors ← neighbors[0 : max_neighbors]
23.
24.     neighbor_lists[i] ← neighbors

25. return neighbor_lists
```

**Complexity**:

- Cloud size: O(N · (2n+1)³) where n = num images
- Build KD-tree: O(cloud_size · log(cloud_size))
- Query per atom: O(log(cloud_size) + k) where k = neighbors found
- Total: O(N · (2n+1)³ · log(N · (2n+1)³))

For typical structures (N ≈ 50, n ≈ 2), cloud_size ≈ 6250, which is very manageable.

---

### 3.4 Edge Features

**File**: `src/graph/edge_features.cpp`

```
GAUSSIAN-RBF(d, r_cutoff, Δr)
─────────────────────────────
Input:  d ∈ ℝ         (distance)
        r_cutoff ∈ ℝ  (default 10.0)
        Δr ∈ ℝ        (default 0.1)
Output: g ∈ ℝⁿ        (100-dim RBF vector)

1.  n ← ⌊r_cutoff / Δr⌋
2.  σ ← r_cutoff / 3
3.  inv_σ² ← 1 / σ²
4.  norm ← 1 / (σ · √(2π))

5.  for k ← 0 to n−1 do
6.      center ← k · Δr
7.      g[k] ← norm · exp(−0.5 · (center − d)² · inv_σ²)

8.  return g
```

**Output shape**: For each edge, we get a 100-dimensional vector.

---

### 3.5 Crystal Graph

**File**: `src/graph/crystal_graph.cpp`

This ties everything together.

```
CRYSTAL-GRAPH-INIT(S, neighbors, embeddings, r_cutoff)
──────────────────────────────────────────────────────
Input:  S (Structure with N atoms)
        neighbors (NeighborList)
        embeddings (map: atomic_number → 92-dim vector)
        r_cutoff ∈ ℝ
Output: G (CrystalGraph with node_features, edge_index, edge_attr)

▷ Step 1: Build node features [N × 92]
1.  feat_dim ← 92
2.  X ← N × feat_dim matrix

3.  for i ← 0 to N−1 do
4.      Z ← S.atoms[i].element          ▷ Atomic number
5.      X[i] ← embeddings[Z]

▷ Step 2: Count total edges
6.  E ← 0
7.  for i ← 0 to N−1 do
8.      E ← E + |neighbors[i]|

▷ Step 3: Build edge_index [2 × E] and edge_attr [E × 100]
9.  edge_index ← 2 × E matrix
10. n_rbf ← ⌊r_cutoff / 0.1⌋            ▷ 100 bins
11. edge_attr ← E × n_rbf matrix

12. e ← 0                                ▷ Edge counter
13. for i ← 0 to N−1 do
14.     for each nbr in neighbors[i] do
15.         edge_index[0, e] ← i         ▷ Source
16.         edge_index[1, e] ← nbr.idx   ▷ Target
17.         edge_attr[e] ← GAUSSIAN-RBF(nbr.distance, r_cutoff)
18.         e ← e + 1

19. G.node_features ← X
20. G.edge_index ← edge_index
21. G.edge_attr ← edge_attr
22. G.target ← 0                         ▷ Set later via SET-TARGET

23. return G


ADD-TOPO-FEATURES(G, topo)
──────────────────────────
Input:  G (CrystalGraph with node_features [N × 92])
        topo [N × n_pca] (PCA-reduced Betti features)
Output: G with node_features [N × (92 + n_pca)]

1.  G.node_features ← CONCAT(G.node_features, topo, axis=1)
2.  return G
```

---

## Summary: Function Call Order

```
MAIN-PIPELINE(poscar_path, embeddings_path, csv_path, idx)
──────────────────────────────────────────────────────────
Input:  poscar_path (path to VASP POSCAR)
        embeddings_path (path to atomic_embedding_CGCNN.json)
        csv_path (path to data.csv)
        idx (index into CSV for this structure)
Output: G (CrystalGraph ready for GNN)

1.  V ← PARSE-VASP(poscar_path)
2.  embeddings ← PARSE-ATOM-EMBEDDINGS(embeddings_path)
3.  defects ← PARSE-DEFECT-CSV(csv_path)

4.  S ← STRUCTURE-INIT(V)

5.  neighbors ← BUILD-NEIGHBOR-LIST-WITH-PBC(S, r_cutoff=10.0, max_neighbors=20)

6.  G ← CRYSTAL-GRAPH-INIT(S, neighbors, embeddings, r_cutoff=10.0)

7.  G.target ← defects[idx].formation_energy

8.  return G                             ▷ Ready for GNN!
```

---

## Testing Checklist

- [ ] `parse_vasp`: Verify lattice matches pymatgen load
- [ ] `Structure`: Verify `distance(i,j)` matches pymatgen with PBC
- [ ] `NeighborList`: Compare neighbor counts and distances vs pymatgen `get_all_neighbors`
- [ ] `gaussian_rbf`: Verify output matches Python `calculateEdgeAttributes`
- [ ] `CrystalGraph`: Verify shapes match PyTorch Geometric `Data` object

---

## Dependencies

| Library       | Purpose      | Install     |
| ------------- | ------------ | ----------- |
| Eigen         | Matrix math  | Header-only |
| nanoflann     | KD-tree      | Header-only |
| nlohmann/json | JSON parsing | Header-only |

All header-only, making WASM compilation straightforward.
