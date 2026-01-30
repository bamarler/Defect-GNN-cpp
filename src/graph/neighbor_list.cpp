#include "graph/neighbor_list.hpp"

#include "crystal/structure.hpp"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <nanoflann.hpp>
#include <vector>

namespace defect_gnn::graph {

NeighborList::NeighborList(const crystal::Structure& structure,
                           double r_cutoff,  // NOLINT(bugprone-easily-swappable-parameters)
                           size_t max_neighbors,
                           double epsilon)
    : r_cutoff_(r_cutoff), max_neighbors_(max_neighbors), epsilon_(epsilon) {
    neighbor_lists_.resize(structure.num_atoms());
    build_with_pbc(structure);
}

const std::vector<Neighbor>& NeighborList::neighbors(size_t atom_idx) const {
    return neighbor_lists_[atom_idx];
}

void NeighborList::build_with_pbc(const crystal::Structure& structure) {
    int num_images = compute_num_images(structure.lattice(), r_cutoff_);
    PointCloud cloud = create_image_cloud(structure, num_images);

    NeighborList::KDTree tree =
        NeighborList::KDTree(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    tree.buildIndex();

    for (size_t i = 0; i < structure.num_atoms(); i++) {
        Eigen::Vector3d query_pt = structure.atoms()[i].position;

        std::vector<nanoflann::ResultItem<unsigned int, double>> matches;

        tree.radiusSearch(query_pt.data(), r_cutoff_ * r_cutoff_, matches);

        std::vector<Neighbor> neighbor_list;

        for (const nanoflann::ResultItem<unsigned int, double>& match : matches) {
            size_t orig_idx = cloud.original_index(match.first);

            if (orig_idx == i && std::sqrt(match.second) < epsilon_) {
                continue;
            }

            auto delta_r = cloud.position(match.first) - query_pt;

            neighbor_list.push_back(Neighbor(orig_idx, std::sqrt(match.second), delta_r));
        }

        std::sort(neighbor_list.begin(),
                  neighbor_list.end(),
                  [](const Neighbor& a, const Neighbor& b) { return a.distance < b.distance; });

        if (neighbor_list.size() > max_neighbors_) {
            neighbor_list.resize(max_neighbors_);
        }

        neighbor_lists_[i] = neighbor_list;
    }
}

int NeighborList::compute_num_images(const Eigen::Matrix3d& lattice, double r_cutoff) {
    double l_min = std::min({lattice.row(0).norm(), lattice.row(1).norm(), lattice.row(2).norm()});

    return static_cast<int>(std::ceil(r_cutoff / l_min)) + 1;
}

auto NeighborList::create_image_cloud(const crystal::Structure& structure,
                                      int num_images) -> PointCloud {
    PointCloud cloud;
    Eigen::Matrix3d lattice = structure.lattice();

    for (int n_a = -num_images; n_a <= num_images; n_a++) {
        for (int n_b = -num_images; n_b <= num_images; n_b++) {
            for (int n_c = -num_images; n_c <= num_images; n_c++) {
                Eigen::Vector3d offset =
                    (n_a * lattice.row(0) + n_b * lattice.row(1) + n_c * lattice.row(2))
                        .transpose();

                for (int i = 0; i < static_cast<int>(structure.num_atoms()); i++) {
                    cloud.add_point(structure.atoms()[i].position + offset, i);
                }
            }
        }
    }

    return cloud;
}

}  // namespace defect_gnn::graph