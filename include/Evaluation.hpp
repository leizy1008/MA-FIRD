/**
 * Evaluation.hpp
 * created on : September 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_EVALUATION_HPP
#define MA_FIRD_EVALUATION_HPP

#include <iostream>
#include "Config.hpp"
#include "Route.hpp"

class EvalElem {
public:
    double cost = 0, duration_time = 0, open_time = 0, close_time = 0, wait_time = 0, reverse_time = 0,
        in_load = 0, out_load = 0, max_load = 0;
    EvalElem() = default;
    ~EvalElem() = default;
    explicit EvalElem(Node *node);
    EvalElem(Route *route, int start_idx, int end_idx);
    EvalElem add(EvalElem &ee, double dist, double time_dist) const;
};

class EvalResult {
public:
    Route *ri = nullptr, *rj = nullptr;
    Node *pi = nullptr, *pj = nullptr;
    SuperNode spi, spj;
    std::string type = "";
    double score = INF;
    double cost_delta = 0, time_delta = 0, load_delta = 0;
    double cost_value = 0, time_value = 0, load_value = 0;
    double cost_oi = 0, cost_oj = 0, cost_ni = 0, cost_nj = 0;
    double time_oi = 0, time_oj = 0, time_ni = 0, time_nj = 0;
    double load_oi = 0, load_oj = 0, load_ni = 0, load_nj = 0;
    bool accepted = false;
    bool empty = true;
    bool done = false;
    int count = 0;
};

#endif //MA_FIRD_EVALUATION_HPP
