/**
 * Generator.hpp
 * created on : August 2023
 * author : Z.LEI
 **/


#ifndef MA_FIRD_GENERATOR_HPP
#define MA_FIRD_GENERATOR_HPP

#include <iostream>
#include "Config.hpp"
#include "Random.hpp"
#include "Parameters.hpp"
#include "Solution.hpp"
#include "RLAgent.hpp"

class Generator {
private:
    static Random *random;
    static Parameters *params;
    static Data *data;
    RLAgent parent_agent, insert_agent;
    int n_parents = 2;
    int max_parents_num = 10;
    int insert_opt = 0;
    double ratio = 0.9;
    int step = 0;
    double abs_sum_reward = 0;
    Solution* ARIX(const std::vector<Solution*>& parents);
public:
    Generator() = default;
    ~Generator() = default;
    static void setContext(Random *_random, Parameters *_params, Data *_data);
    void init();
    std::vector<int> selectParents(int pop_size);
    Solution* breed(std::vector<Solution*> &parents);
    void update(std::vector<Solution*> &parents, Solution* offspring);
};


#endif //MA_FIRD_GENERATOR_HPP
