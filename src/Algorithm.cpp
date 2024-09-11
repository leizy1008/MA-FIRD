/**
 * Algorithm.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Algorithm.hpp"

Algorithm::Algorithm(Parameters *params) {
    this->params = params;
    this->random = new Random(params->seed);
    this->data = new Data(params);
    this->best_solution = nullptr;
}

Algorithm::~Algorithm() {
    delete params;
    delete random;
    delete data;
    delete best_solution;
}

void Algorithm::setContext() {
    Population::setContext(random, params, data);
    Solution::setContext(random, params, data);
    Route::setContext(random, params, data);
}

void Algorithm::run() {
    data->read();
    this->setContext();
    // start
    auto start = std::chrono::high_resolution_clock::now();
    // initial population
    population.init();
    best_solution = new Solution(*population.best_solution);
    if (LOG_LEVEL > 0) best_solution->write(0);
    // iteration
    int iter = 0, best_iter = 0, stagnation = 0;
    double best_touch_time = 0, running_time = 0;
    while (iter++ < params->max_iterations &&
        stagnation < params->patience &&
        running_time < params->max_running_time
    ) {
        auto start_iter = std::chrono::high_resolution_clock::now();
        // next generation
        population.next();
        auto new_solution = population.best_solution;
        auto end_iter = std::chrono::high_resolution_clock::now();
        running_time = std::chrono::duration<double> (end_iter - start).count();

        if (new_solution->betterThan(best_solution)) {
            delete best_solution;
            best_solution = new Solution(*new_solution);
            best_iter = iter;
            best_touch_time = running_time;
            stagnation = 0;
            if (LOG_LEVEL > 0) best_solution->write(iter);
        } else {
            stagnation++;
        }

        std::cout << "[LOG]"
                  << " iter: " << iter
                  << " best_iter: " << best_iter
                  << " nb_vehicles: " << best_solution->nb_vehicle
                  << " best_cost: " << best_solution->cost
                  << " best_total_cost: " << best_solution->total_cost
                  << " iter_time: " << std::chrono::duration<double> (end_iter - start_iter).count()
                  << " running_time: " << running_time
                  << std::endl << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();

    best_solution->print();
    if (LOG_LEVEL == 0) best_solution->write(best_iter);

    std::cout << "[SUMMARY]"
              << " instance: " << params->filename
              << " nb_vehicles: " << best_solution->nb_vehicle
              << " best_cost: " << best_solution->cost
              << " best_total_cost: " << best_solution->total_cost
              << " running_time: " << running_time
              << " best_touch_time: " << best_touch_time
              << " solution_file: " << params->filename + "-" + params->timestamp + "-"+ std::to_string(best_iter)
              << std::endl << std::endl;
}

