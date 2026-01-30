#pragma once

#include "crystal/structure.hpp"

#include <Eigen/Dense>

#include <nanoflann.hpp>
#include <vector>

namespace defect_gnn::graph {
struct Neighbor {
    size_t idx;
    double distance;
    Eigen::Vector3d displacement;
};

class NeighborList {
public:
    explicit NeighborList(const crystal::Structure& structure,
                          double r_cutoff = 10.0,
                          size_t max_neighbors = 20,
                          double epsilon = 1e-10);

    [[nodiscard]] const std::vector<Neighbor>& neighbors(size_t atom_idx) const;

private:
    class PointCloud {
    public:
        [[nodiscard]] size_t kdtree_get_point_count() const { return positions_.size(); };
        [[nodiscard]] double kdtree_get_pt(size_t idx, size_t dim) const {
            return positions_[idx][static_cast<int>(dim)];
        };

        template <class BBox>
        bool kdtree_get_bbox(BBox& /* bb */) const {
            return false;
        }

        void add_point(const Eigen::Vector3d& position, size_t original_index) {
            positions_.push_back(position);
            original_indices_.push_back(original_index);
        }

        void reserve(size_t n) {
            positions_.reserve(n);
            original_indices_.reserve(n);
        }

        [[nodiscard]] const Eigen::Vector3d& position(size_t idx) const { return positions_[idx]; }
        [[nodiscard]] size_t original_index(size_t idx) const { return original_indices_[idx]; }

    private:
        std::vector<Eigen::Vector3d> positions_;
        std::vector<size_t> original_indices_;
    };

    using KDTree = nanoflann::
        KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<double, PointCloud>, PointCloud, 3>;

    [[nodiscard]] static int compute_num_images(const Eigen::Matrix3d& lattice, double r_cutoff);
    [[nodiscard]] static PointCloud create_image_cloud(const crystal::Structure& structure,
                                                       int num_images);

    void build_with_pbc(const crystal::Structure& structure);

    double r_cutoff_;
    size_t max_neighbors_;
    double epsilon_;

    std::vector<std::vector<Neighbor>> neighbor_lists_;
};

}  // namespace defect_gnn::graph