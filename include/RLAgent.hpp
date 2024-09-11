/**
 * RLAgent.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_RLAGENT_HPP
#define MA_FIRD_RLAGENT_HPP

#include <vector>
#include "Random.hpp"
#include "Data.hpp"
#include "Parameters.hpp"
#include "Solution.hpp"

class RLAgent {
private:
    static Random *random;
    static Parameters *params;
    static Data *data;
    int count_sum = 0;
    std::vector<std::vector<double>> q_values;
    std::vector<int> counts;
public:
    RLAgent() = default;
    ~RLAgent() = default;
    static void setContext(Random *_random, Parameters *_params, Data *_data);
    void init(int _num_actions, int _num_states);
    int selectAction(int state);
    void updateQValues(int state, int action, int next_state, double reward);
};


#endif //MA_FIRD_RLAGENT_HPP
