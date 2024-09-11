/**
 * Config.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_CONFIG_HPP
#define MA_FIRD_CONFIG_HPP

#include <string>
#include <vector>

const std::string DATASET_DIR = "./datasets/";
const std::string OUTPUT_DIR = "./output/";

const int DATASETS_INDEX = 1;
const int INSTANCE_INDEX = 0;

const int SEED = 0;
const double PRECISION = 1e-6;
const double INF = 1e10;
const int LOG_LEVEL = 1;

const double DISPATCH_COST = 2000;
const double UNIT_COST = 1;
const int MAX_ITERATIONS = 5000;
const int MAX_RUNNING_TIME = 7200;
const int PATIENCE = 500;
const int POPULATION_SIZE = 10;
const double INIT_RATIO = 0.9;
const double FIT_COEF = 0.8;
const double ADJUST_FACTOR = 0.5;

const std::vector<std::string> DATASETS = {
    "WC_small/",
    "WC_medium/",
    "JD/",
};

const std::vector<std::vector<std::string>> FILENAMES = {
    {"rcdp0501", "rcdp0504", "rcdp0507", "rcdp1001", "rcdp1004", "rcdp1007",
     "rcdp2501", "rcdp2504", "rcdp2507", "rcdp5001", "rcdp5004", "rcdp5007"}, // 0-11
    {"cdp101", "cdp102", "cdp103", "cdp104", "cdp105", "cdp106", "cdp107", "cdp108", "cdp109", // 0-8
    "cdp201", "cdp202", "cdp203", "cdp204", "cdp205", "cdp206", "cdp207", "cdp208", // 9-16
    "rdp101", "rdp102", "rdp103", "rdp104", "rdp105", "rdp106", "rdp107", "rdp108", "rdp109", "rdp110", "rdp111", "rdp112", // 17-28
    "rdp201", "rdp202", "rdp203", "rdp204", "rdp205", "rdp206", "rdp207", "rdp208", "rdp209", "rdp210", "rdp211", // 29-39
    "rcdp101", "rcdp102", "rcdp103", "rcdp104", "rcdp105", "rcdp106", "rcdp107", "rcdp108", // 40-47
    "rcdp201", "rcdp202", "rcdp203", "rcdp204", "rcdp205", "rcdp206", "rcdp207", "rcdp208"},// 48-55
    {"200_1", "200_2", "200_3", "200_4", // 0-3
    "400_1", "400_2", "400_3", "400_4", // 4-7
    "600_1", "600_2", "600_3", "600_4", // 8-11
    "800_1", "800_2", "800_3", "800_4", // 12-15
    "1000_1", "1000_2", "1000_3", "1000_4"} // 16-19
};

#endif //MA_FIRD_CONFIG_HPP

