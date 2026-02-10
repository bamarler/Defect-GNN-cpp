
#include "crystal/structure.hpp"
#include "io/vasp_parser.hpp"
#include "topology/betti_features.hpp"
#include "topology/pca.hpp"
#include "utils/logging.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace defect_gnn::preprocess {

// Parse "X_Y" filename format into (structure_num, defect_num)
std::pair<int, int> parse_structure_id(const std::string& id) {
    size_t underscore = id.find('_');
    if (underscore == std::string::npos) {
        return {std::stoi(id), 0};
    }
    return {std::stoi(id.substr(0, underscore)), std::stoi(id.substr(underscore + 1))};
}

void preprocess_all_structure(const std::string& raw_path,  // NOLINT(readability-function-size)
                              const std::string& processed_path,
                              double r_cutoff = 10,  // NOLINT(bugprone-easily-swappable-parameters)
                              int n_pca_components = 6,
                              unsigned num_threads = 8) {
    std::vector<std::string> structure_ids;

    for (const auto& entry : fs::directory_iterator(raw_path)) {
        if (entry.path().extension() == ".vasp") {
            structure_ids.push_back(entry.path().stem().string());
        }
    }

    std::sort(
        structure_ids.begin(), structure_ids.end(), [](const std::string& a, const std::string& b) {
            return parse_structure_id(a) < parse_structure_id(b);
        });

    std::map<int, int> defects_per_structure;
    for (const auto& id : structure_ids) {
        auto [struct_num, defect_num] = parse_structure_id(id);
        defects_per_structure[struct_num]++;
    }

    spdlog::info("Found {} defective structures from {} base structures",
                 structure_ids.size(),
                 defects_per_structure.size());

    std::vector<Eigen::MatrixXd> all_structure_features;
    int current_structure_num = -1;

    for (size_t i = 0; i < structure_ids.size(); ++i) {
        const std::string& structure_id = structure_ids[i];
        auto [struct_num, defect_num] = parse_structure_id(structure_id);

        if (struct_num != current_structure_num) {
            current_structure_num = struct_num;
            spdlog::info("[{}/{}] Processing structure {} ({} defects)",
                         i + 1,
                         structure_ids.size(),
                         struct_num,
                         defects_per_structure[struct_num]);
        }

        io::VASPStructure vasp = io::parse_vasp(std::format("{}/{}.vasp", raw_path, structure_id));

        crystal::Structure structure(vasp);

        Eigen::MatrixXd features =
            topology::compute_structure_betti_features(structure, r_cutoff, num_threads);

        topology::save_betti_features(std::format("{}/betti/{}.bin", processed_path, structure_id),
                                      features);

        all_structure_features.push_back(features);
    }

    size_t total_atoms = 0;
    for (const Eigen::MatrixXd& f : all_structure_features) {
        total_atoms += f.rows();
    }

    Eigen::MatrixXd all_features(total_atoms, topology::BETTI_FEATURE_DIM);
    size_t row = 0;
    for (const Eigen::MatrixXd& f : all_structure_features) {
        all_features.middleRows(static_cast<int>(row), f.rows()) = f;
        row += f.rows();
    }

    spdlog::info("Fitting PCA on {} atoms...", total_atoms);
    topology::PCA pca;
    pca.fit(all_features, n_pca_components);
    pca.save(processed_path + "/pca_model.bin");

    spdlog::info("Processed {} structures", structure_ids.size());
    spdlog::info("Total atoms: {}", all_features.rows());
    spdlog::info("PCA explained variance ratio: {}", pca.explained_variance_ratio().sum());
}

}  // namespace defect_gnn::preprocess

int main(int argc, char** argv) {
    defect_gnn::utils::init_logger("preprocess_betti");

    // Default paths
    std::string raw_path = "data/raw/defective_structures";
    std::string processed_path = "data/processed";
    double r_cutoff = 10;
    int n_pca_components = 6;
    unsigned num_threads = 8;

    // Parse command line args (optional overrides)
    if (argc >= 3) {
        raw_path = argv[1];
        processed_path = argv[2];
    }
    if (argc >= 4) {
        r_cutoff = std::stod(argv[3]);
    }
    if (argc >= 5) {
        n_pca_components = std::stoi(argv[4]);
    }

    // Create output directories
    fs::create_directories(processed_path + "/betti");

    spdlog::info("Preprocessing Betti features...");
    spdlog::info("  Raw path: {}", raw_path);
    spdlog::info("  Output path: {}", processed_path);
    spdlog::info("  r_cutoff: {}", r_cutoff);
    spdlog::info("  PCA components: {}", n_pca_components);
    spdlog::info("  Number of Threads: {}", num_threads);

    defect_gnn::preprocess::preprocess_all_structure(
        raw_path, processed_path, r_cutoff, n_pca_components, num_threads);

    spdlog::info("Done!");
    return 0;
}