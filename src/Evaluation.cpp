/**
 * Evaluation.cpp
 * created on : September 2023
 * author : Z.LEI
 **/

#include "Evaluation.hpp"

EvalElem::EvalElem(Node *node) {
    duration_time = node->service_time;
    open_time = node->open_time;
    close_time = node->close_time;
    in_load = node->delivery;
    out_load = node->pickup;
    max_load = std::max(in_load, out_load);
    wait_time = 0;
    reverse_time = 0;
}

EvalElem::EvalElem(Route *route, int start_idx, int end_idx) {
    cost = route->cost_matrix[start_idx][end_idx];
    duration_time = route->duration_time_matrix[start_idx][end_idx];
    open_time = route->open_time_matrix[start_idx][end_idx];
    close_time = route->close_time_matrix[start_idx][end_idx];
    wait_time = route->wait_time_matrix[start_idx][end_idx];
    reverse_time = route->reverse_time_matrix[start_idx][end_idx];
    in_load = route->in_load_matrix[start_idx][end_idx];
    out_load = route->out_load_matrix[start_idx][end_idx];
    max_load = route->max_load_matrix[start_idx][end_idx];
}

EvalElem EvalElem::add(EvalElem &ee, double dist, double time_dist) const {
    EvalElem res;
    // cost
    res.cost = cost + dist + ee.cost;
    // time windows
    double td = duration_time + wait_time + time_dist - reverse_time;
    double wt = std::max(ee.open_time - td - close_time, 0.0);
    double rt = std::max(open_time + td - ee.close_time, 0.0);
    res.duration_time = duration_time + time_dist + ee.duration_time;
    res.open_time = std::max(open_time, ee.open_time - td) - wt;
    res.close_time = std::min(close_time, ee.close_time - td) + rt;
    res.reverse_time = reverse_time + rt + ee.reverse_time;
    res.wait_time = wait_time + wt + ee.wait_time;
    // capacity
    res.in_load = in_load + ee.in_load;
    res.out_load = out_load + ee.out_load;
    res.max_load = std::max(max_load + ee.in_load, out_load + ee.max_load);
    return res;
}