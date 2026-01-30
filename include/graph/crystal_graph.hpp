#pragma once

#include "crystal/structure.hpp"
#include "graph/neighbor_list.hpp"

#include <Eigen/Dense>

#include <map>

namespace defect_gnn::graph {

class CrystalGraph {
public:
    CrystalGraph(const crystal::Structure& structure,
                 const NeighborList& neighbors,
                 const std::map<int, Eigen::VectorXd>& atom_embeddings,
                 int atom_embedding_dims = 92,
                 double r_cutoff = 10,
                 double dr = 0.1);

    [[nodiscard]] const Eigen::MatrixXd& node_features() const;

    [[nodiscard]] const Eigen::MatrixXi& edge_index() const;

    [[nodiscard]] const Eigen::MatrixXd& edge_attr() const;

    [[nodiscard]] double target() const;
    void set_target(double y);

    void add_topo_features(const Eigen::MatrixXd& topo);

    [[nodiscard]] size_t num_nodes() const;
    [[nodiscard]] size_t num_edges() const;

private:
    Eigen::MatrixXd node_features_;
    Eigen::MatrixXi edge_index_;
    Eigen::MatrixXd edge_attr_;
    double target_;
};

}  // namespace defect_gnn::graph