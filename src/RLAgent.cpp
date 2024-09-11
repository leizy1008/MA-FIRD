/**
 * RLAgent.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "RLAgent.hpp"

Random *RLAgent::random = nullptr;
Parameters *RLAgent::params = nullptr;
Data *RLAgent::data = nullptr;

void RLAgent::setContext(Random *_random, Parameters *_params, Data *_data) {
    RLAgent::random = _random;
    RLAgent::data = _data;
    RLAgent::params = _params;
}

void RLAgent::init(int _num_actions, int _num_states) {
    this->q_values = std::vector<std::vector<double>>(_num_states, std::vector<double>(_num_actions, 0.0));
    this->counts = std::vector<int>(_num_actions, 0);
}

int RLAgent::selectAction(int state) {
    auto values = q_values[state];
    auto ucb_values = std::vector<double>(values.size(), 0.0);
    for (int i = 0; i < values.size(); i++) {
        ucb_values[i] = values[i] + std::sqrt(2 * std::log(count_sum) / counts[i] + PRECISION);
    }
    int max_action = std::max_element(ucb_values.begin() ,ucb_values.end()) - ucb_values.begin();
    return max_action;
}

void RLAgent::updateQValues(int state, int action, int next_state, double reward) {
    counts[action] += 1;
    count_sum += 1;
    q_values[state][action] += (reward - q_values[state][action]) / counts[action];
}


