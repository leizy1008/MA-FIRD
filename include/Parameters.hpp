/**
 * Parameters.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_PARAMETERS_HPP
#define MA_FIRD_PARAMETERS_HPP

#include <iostream>
#include <random>
#include <ctime>
#include "cmdline.h"
#include "Config.hpp"

class Parameters {
public:
    int datasets_index;
    int instance_index;
    std::string dataset;
    std::string data_dir;
    std::string output_dir;
    std::string filename;
    std::string timestamp;
    int seed;
    double dispatch_cost;
    double unit_cost;
    double init_ratio;
    int max_iterations;
    int max_running_time;
    int patience;
    int population_size;
    double fit_coef;
    double adjust_factor;
    Parameters(int argc, char **argv);
    Parameters() = default;
    ~Parameters() = default;
    void print() const;
};


#endif //MA_FIRD_PARAMETERS_HPP
