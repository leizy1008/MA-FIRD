/**
 * Searcher.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_SEARCHER_HPP
#define MA_FIRD_SEARCHER_HPP

#include <iostream>
#include <algorithm>
#include <climits>
#include "Random.hpp"
#include "Parameters.hpp"
#include "Data.hpp"
#include "Solution.hpp"
#include "Operator.hpp"
#include "RLAgent.hpp"

class Searcher {
private:
    static Random *random;
    static Parameters *params;
    static Data *data;
    Operator opt;
    std::vector<Neighborhood> neighborhoods;
public:
    Searcher();
    ~Searcher();
    static void setContext(Random *_random, Parameters *_params, Data *_data);
    Solution *FIRD(Solution *solution);
    Solution *FIS(Solution *solution, Solution *objective);
};

#endif //MA_FIRD_SEARCHER_HPP
