/**
 * Algorithm.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_ALGORITHM_HPP
#define MA_FIRD_ALGORITHM_HPP

#include <iostream>
#include <algorithm>
#include <chrono>
#include "Config.hpp"
#include "Random.hpp"
#include "Parameters.hpp"
#include "Data.hpp"
#include "Solution.hpp"
#include "Population.hpp"
#include "Searcher.hpp"

class Algorithm {
private:
    Random *random = nullptr;
    Parameters *params = nullptr;
    Data *data = nullptr;
    Population population;
    Solution *best_solution = nullptr;
public:
    Algorithm(Parameters *params);
    Algorithm() = default;
    ~Algorithm();
    void setContext();
    void run();
};


#endif //MA_FIRD_ALGORITHM_HPP
