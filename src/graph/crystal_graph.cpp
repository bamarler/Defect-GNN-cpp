#include "graph/crystal_graph.hpp"

#include "graph/edge_features.hpp"

#include <Eigen/Dense>

namespace defect_gnn::graph {

CrystalGraph::CrystalGraph(const crystal::Structure& structure,
                           const NeighborList& neighbors,
                           const std::map<int, Eigen::VectorXd>& atom_embeddings,
                           int atom_embedding_dims,  // NOLINT(bugprone-easily-swappable-parameters)
                           double r_cutoff,
                           double dr) {
    int num_atoms = static_cast<int>(structure.num_atoms());

    node_features_.resize(num_atoms, atom_embedding_dims);

    for (int i = 0; i < num_atoms; i++) {
        node_features_.row(i) = atom_embeddings.at(structure.atoms()[i].element);
    }

    int edge_count = 0;
    for (int i = 0; i < num_atoms; i++) {
        edge_count += static_cast<int>(neighbors.neighbors(i).size());
    }

    edge_index_.resize(2, edge_count);
    int n_rbf = static_cast<int>(std::floor(r_cutoff / dr));
    edge_attr_.resize(edge_count, n_rbf);

    int edge = 0;
    for (int i = 0; i < num_atoms; i++) {
        for (const Neighbor& neighbor : neighbors.neighbors(i)) {
            edge_index_(0, edge) = i;
            edge_index_(1, edge) = static_cast<int>(neighbor.idx);
            edge_attr_.row(edge) = defect_gnn::graph::gaussian_rbf(neighbor.distance, r_cutoff, dr);
            edge++;
        }
    }

    target_ = 0;
}

const Eigen::MatrixXd& CrystalGraph::node_features() const {
    return node_features_;
}

const Eigen::MatrixXi& CrystalGraph::edge_index() const {
    return edge_index_;
}

const Eigen::MatrixXd& CrystalGraph::edge_attr() const {
    return edge_attr_;
}

double CrystalGraph::target() const {
    return target_;
}

void CrystalGraph::set_target(double y) {
    target_ = y;
}

void CrystalGraph::add_topo_features(const Eigen::MatrixXd& topo) {
    // bamarler TODO: implement this
}

size_t CrystalGraph::num_nodes() const {
    return node_features_.rows();
}

size_t CrystalGraph::num_edges() const {
    return edge_index_.cols();
}

}  // namespace defect_gnn::graph