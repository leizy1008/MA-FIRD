/**
 * Parameters.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Parameters.hpp"

Parameters::Parameters(int argc, char **argv) {
    std::random_device rand_dev;
    cmdline::parser parser;
    parser.add<std::string>("dataset_dir", 0, "datasets dir", false, "");
    parser.add<std::string>("output_dir", 0, "output dir", false, "");
    parser.add<int>("datasets", 'D', "datasets index", false, DATASETS_INDEX);
    parser.add<int>("instance", 'I', "instance index", false, INSTANCE_INDEX);
    parser.add<int>("seed", 'S', "seed for random generator", false, SEED);
    parser.add<double>("dispatch_cost", 'c', "dispatch cost", false, DISPATCH_COST);
    parser.add<double>("unit_cost", 'v', "unicost", false, UNIT_COST);
    parser.add<int>("max_iterations", 't', "max iteration", false, MAX_ITERATIONS);
    parser.add<int>("max_running_time", 'e', "max running time", false, MAX_RUNNING_TIME);
    parser.add<int>("patience", 'p', "patience for stagnation iterations", false, PATIENCE);
    parser.add<int>("population_size", 'z', "population size", false, POPULATION_SIZE);
    parser.add<double>("init_ratio", 'r', "init ratio for ARIX", false, INIT_RATIO);
    parser.add<double>("fit_coef", 'y', "fitness coefficient", false, FIT_COEF);
    parser.add<double>("adjust_factor", 'u', "adjust factor", false, ADJUST_FACTOR);
    parser.parse_check(argc, argv);

    this->datasets_index = parser.get<int>("datasets");
    this->instance_index = parser.get<int>("instance");
    if (this->datasets_index < 0 || this->datasets_index >= DATASETS.size()) {
        throw std::invalid_argument("[ERROR] Invalid data_dir_index: " + std::to_string(this->datasets_index));
    }
    if (this->instance_index < 0 || this->instance_index >= FILENAMES[this->datasets_index].size()) {
        throw std::invalid_argument("[ERROR] Invalid filename_index: " + std::to_string(this->instance_index));
    }
    this->dataset = DATASETS[this->datasets_index];

    if (parser.exist("dataset_dir") && parser.exist("output_dir")) {
        this->data_dir = parser.get<std::string>("dataset_dir") + this->dataset;
        this->output_dir = parser.get<std::string>("output_dir");
    } else {
        this->data_dir = DATASET_DIR + this->dataset;
        this->output_dir = OUTPUT_DIR;
    }

    this->filename = FILENAMES[this->datasets_index][this->instance_index];
    this->seed = parser.get<int>("seed");
    if (this->seed == 0) {
        this->seed = rand_dev();
    }
    this->timestamp = std::to_string(std::time(nullptr));
    this->dispatch_cost = parser.get<double>("dispatch_cost");
    this->unit_cost = parser.get<double>("unit_cost");
    this->init_ratio = parser.get<double>("init_ratio");
    this->max_iterations = parser.get<int>("max_iterations");
    this->max_running_time = parser.get<int>("max_running_time");
    this->patience = parser.get<int>("patience");
    this->population_size = parser.get<int>("population_size");
    this->fit_coef = parser.get<double>("fit_coef");
    this->adjust_factor = parser.get<double>("adjust_factor");
}

void Parameters::print() const {
    std::cout << "[INFO]"
                << " instance: " << filename
                << " data_dir: " << data_dir
                << " output_dir: " << output_dir
                << " timestamp: " << timestamp
                << " seed: " << seed
                << " max_iterations: " << max_iterations
                << " max_running_time: " << max_running_time
                << " patience: " << patience
                << " dispatch_cost: " << dispatch_cost
                << " unit_cost: " << unit_cost
                << " population_size: " << population_size
                << " fit_coef: " << fit_coef
                << " init_ratio: " << init_ratio
                << " adjust_factor: " << adjust_factor
                << std::endl
                << std::endl;
}
