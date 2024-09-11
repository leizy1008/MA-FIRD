/**
 * Operator.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Operator.hpp"

Random *Operator::random = nullptr;
Parameters *Operator::params = nullptr;
Data *Operator::data = nullptr;

void Operator::setContext(Random *_random, Parameters *_params, Data *_data) {
    Operator::random = _random;
    Operator::params = _params;
    Operator::data = _data;
}

void Operator::init(bool flag) {
    adjust_factor = params->adjust_factor;
    if (flag) {
        // solution is feasible
        w1 = min_pr, w2 = min_pr;

    } else {
        // solution is not feasible, repair
        w1 = max_pr, w2 = max_pr;
    }
    cost_coef = w0;
    time_coef = random->randomDouble(0.5, 1.5) * w1;
    load_coef = random->randomDouble(0.5, 1.5) * w2;
}

void Operator::update(Solution *solution) {
    if (solution->tw_feasible) {
        w1 = std::max(w1 * adjust_factor, min_pr);
    } else {
        w1 = std::min(w1 / adjust_factor, max_pr);
    }
    if (solution->cap_feasible) {
        w2 = std::max(w2 * adjust_factor, min_pr);
    } else {
        w2 = std::min(w2 / adjust_factor, max_pr);
    }
    time_coef = random->randomDouble(0.5, 1.5) * w1;
    load_coef = random->randomDouble(0.5, 1.5) * w2;
}

bool Operator::acceptMove(EvalResult &res) {
    if (! res.empty && res.score < -PRECISION) return true;
    else return false;
}

bool Operator::betterMove(EvalResult &res, double score) {
    return std::abs(score) > PRECISION && score < res.score;
}

bool Operator::skip(int rid, int id1, int id2) {
    if (!data->feasibility_matrix[id1][id2]) return true;
    return false;
}

double Operator::getScore(double cost_delta, double time_delta, double load_delta) const {
    return cost_coef * cost_delta +
           time_coef * data->dist_time_ratio * time_delta +
           load_coef * data->std_dist_delta * load_delta / data->capacity;
}

void Operator::_intraRelocate(Solution *solution, Route *route, int len, EvalResult &res) {
    auto nodes = route->nodes;
    int size = nodes.size();
    auto indices = random->randomInts(size);

    for (auto i : indices) {
        if (i >= (size + 1 - len)) continue;
        if (nodes[i]->id == 0) continue;
        for (auto j : indices) {
            if (i <= j && j <= (i + len - 1)) continue;
            if (nodes[i]->id == nodes[j]->id || nodes[j]->next->id == nodes[i]->id) continue;
            res.count++;
            Node *first_i = nodes[i], *last_i = nodes[i+len-1], *pj = nodes[j];
            Node *prev_i = first_i->prev, *prev_j = pj->prev, *next_i = last_i->next, *next_j = pj->next;
            if (skip(route->id, pj->id, first_i->id) || skip(route->id, last_i->id, next_j->id) ||
                skip(route->id, prev_i->id, next_i->id)) continue;
            double dist4 = data->distances[prev_i->id][next_i->id], dist5 = data->distances[pj->id][first_i->id], dist6 = data->distances[last_i->id][next_j->id];
            double time_dist4 = data->time_distances[prev_i->id][next_i->id], time_dist5 = data->time_distances[pj->id][first_i->id], time_dist6 = data->time_distances[last_i->id][next_j->id];
            EvalElem ee;
            if (i < j) {
                // (0, i-1) (i, i+len-1) (i+len, j) (j+1, size)
                EvalElem ee1 = EvalElem(route, 0, i-1), ee2 = EvalElem(route, i, i+len-1), ee3 = EvalElem(route, i+len, j), ee4 = EvalElem(route, j+1, size);
                ee = ee1.add(ee3, dist4, time_dist4).add(ee2, dist5, time_dist5).add(ee4, dist6, time_dist6);
            } else {
                // (0, j) (j+1, i-1) (i, i+len-1) (i+len, size)
                EvalElem ee1 = EvalElem(route, 0, j), ee2 = EvalElem(route, j+1, i-1), ee3 = EvalElem(route, i, i+len-1), ee4 = EvalElem(route, i+len, size);
                ee = ee1.add(ee3, dist5, time_dist5).add(ee2, dist6, time_dist6).add(ee4, dist4, time_dist4);
            }

            double overload = std::max(ee.max_load - data->capacity, 0.0);
            bool feasible = ee.reverse_time < PRECISION && overload < PRECISION;

            double cost_delta = ee.cost - route->cost;
            double time_delta = ee.reverse_time - route->reverse_time;
            double load_delta = overload - route->overload;
            double score = getScore(cost_delta, time_delta, load_delta);

            if (betterMove(res, score)) {
                res.type = "INTRA_RELOCATE_" + std::to_string(len);
                res.empty = false;
                res.score = score;
                res.ri = route;
                res.rj = nullptr;
                res.spi = SuperNode(first_i, last_i, len);
                res.pj = pj;
                if (LOG_LEVEL >= 1) {
                    res.cost_delta = cost_delta;
                    res.time_delta = time_delta;
                    res.load_delta = load_delta;
                    res.cost_value = cost_coef * cost_delta;
                    res.time_value = time_coef * time_delta * data->dist_time_ratio;
                    res.load_value = load_coef * data->std_dist_delta * load_delta / data->capacity;
                    res.cost_oi = route->cost;
                    res.time_oi = route->reverse_time;
                    res.load_oi = route->overload;
                    res.cost_ni = ee.cost;
                    res.time_ni = ee.reverse_time;
                    res.load_ni = overload;
                    res.cost_oj = 0;
                    res.time_oj = 0;
                    res.load_oj = 0;
                    res.cost_nj = 0;
                    res.time_nj = 0;
                    res.load_nj = 0;
                }
            }
        }
    }
}

void Operator::_intraSwap(Solution *solution, Route *route, int len_i, int len_j, EvalResult &res) {
    auto nodes = route->nodes;
    int size = nodes.size();
    auto indices = random->randomInts(size);

    for (auto i : indices) {
        if (i >= (size + 1 - len_i)) continue;
        if (nodes[i]->id == 0) continue;
        for (auto j : indices) {
            if (j < i + len_i || j >= (size + 1 - len_j)) continue;
            Node *first_i = nodes[i], *last_i = nodes[i+len_i-1], *first_j = nodes[j], *last_j = nodes[j+len_j-1];
            Node *prev_i = first_i->prev, *prev_j = first_j->prev, *next_i = last_i->next, *next_j = last_j->next;
            if (next_i->id == first_j->id || next_j->id == first_i->id) continue;
            res.count++;
            if (skip(route->id, prev_i->id, first_j->id) || skip(route->id, last_j->id, next_i->id) ||
                skip(route->id, prev_j->id, first_i->id) || skip(route->id, last_i->id, next_j->id)) continue;
            double dist5 = data->distances[prev_i->id][first_j->id], dist6 = data->distances[last_j->id][next_i->id], dist7 = data->distances[prev_j->id][first_i->id], dist8 = data->distances[last_i->id][next_j->id];
            double time_dist5 = data->time_distances[prev_i->id][first_j->id], time_dist6 = data->time_distances[last_j->id][next_i->id], time_dist7 = data->time_distances[prev_j->id][first_i->id], time_dist8 = data->time_distances[last_i->id][next_j->id];
            // (0,i-1) (i,i+len_i-1) (i+len_i,j-1) (j,j+len_j-1) (j+len_j,end)
            EvalElem ee1 = EvalElem(route, 0, i-1), ee2 = EvalElem(route, i, i+len_i-1), ee3 = EvalElem(route, i+len_i, j-1), ee4 = EvalElem(route, j, j+len_j-1), ee5 = EvalElem(route, j+len_j, size);
            EvalElem ee = ee1.add(ee4, dist5, time_dist5).add(ee3, dist6, time_dist6).add(ee2, dist7, time_dist7).add(ee5, dist8, time_dist8);
            double overload = std::max(ee.max_load - data->capacity, 0.0);
            bool feasible = ee.reverse_time < PRECISION && overload < PRECISION;

            double cost_delta = ee.cost - route->cost;
            double time_delta = ee.reverse_time - route->reverse_time;
            double load_delta = overload - route->overload;
            double score = getScore(cost_delta, time_delta, load_delta);

            if (betterMove(res, score)) {
                res.type = "INTRA_SWAP_" + std::to_string(len_i) + "_" + std::to_string(len_j);
                res.empty = false;
                res.score = score;
                res.ri = route;
                res.rj = nullptr;
                res.spi = SuperNode(first_i, last_i, len_i);
                res.spj = SuperNode(first_j, last_j, len_j);
                if (LOG_LEVEL >= 1) {
                    res.cost_delta = cost_delta;
                    res.time_delta = time_delta;
                    res.load_delta = load_delta;
                    res.cost_value = cost_coef * cost_delta;
                    res.time_value = time_coef * time_delta * data->dist_time_ratio;
                    res.load_value = load_coef * data->std_dist_delta * load_delta / data->capacity;
                    res.cost_oi = route->cost;
                    res.time_oi = route->reverse_time;
                    res.load_oi = route->overload;
                    res.cost_ni = ee.cost;
                    res.time_ni = ee.reverse_time;
                    res.load_ni = overload;
                    res.cost_oj = 0;
                    res.time_oj = 0;
                    res.load_oj = 0;
                    res.cost_nj = 0;
                    res.time_nj = 0;
                    res.load_nj = 0;
                }
            }
        }
    }
}

void Operator::_interRelocate(Solution *solution, Route *ri, Route *rj, int len, EvalResult &res) {
    auto nodes_i = ri->nodes;
    auto nodes_j = rj->nodes;
    int size_i = nodes_i.size(), size_j = nodes_j.size();
    auto indices_i = random->randomInts(size_i);
    auto indices_j = random->randomInts(size_j);

    for (auto i : indices_i) {
        if (i >= size_i + 1 - len) continue;
        if (nodes_i[i]->id == 0) continue;
        for (auto j : indices_j) {
            Node *first_i = nodes_i[i], *last_i = nodes_i[i+len-1], *pj = nodes_j[j];
            Node *prev_i = first_i->prev, *prev_j = pj->prev, *next_i = last_i->next, *next_j = pj->next;
            res.count++;
            if (skip(rj->id, pj->id, first_i->id) || skip(rj->id, last_i->id, next_j->id) ||
                skip(ri->id, prev_i->id, next_i->id)) continue;
            double dist4 = data->distances[prev_i->id][next_i->id], dist5 = data->distances[pj->id][first_i->id], dist6 = data->distances[last_i->id][next_j->id];
            double time_dist4 = data->time_distances[prev_i->id][next_i->id], time_dist5 = data->time_distances[pj->id][first_i->id], time_dist6 = data->time_distances[last_i->id][next_j->id];
            EvalElem eei1 = EvalElem(ri, 0, i-1), eei2 = EvalElem(ri, i, i+len-1), eei3 = EvalElem(ri, i+len, ri->size);
            EvalElem eej1 = EvalElem(rj, 0, j), eej2 = EvalElem(rj, j+1, rj->size);
            EvalElem ee1 = eei1.add(eei3, dist4, time_dist4);
            EvalElem ee2 = eej1.add(eei2, dist5, time_dist5).add(eej2, dist6, time_dist6);

            double overload1 = std::max(ee1.max_load - data->capacity, 0.0);
            double overload2 = std::max(ee2.max_load - data->capacity, 0.0);

            // filter infeasible
            bool feasible1 = ee1.reverse_time < PRECISION && overload1 < PRECISION;
            bool feasible2 = ee2.reverse_time < PRECISION && overload2 < PRECISION;

            double cost_delta1 = ee1.cost - ri->cost, cost_delta2 = ee2.cost - rj->cost;
            double cost_delta = cost_delta1 + cost_delta2;
            double time_delta1 = ee1.reverse_time - ri->reverse_time;
            double time_delta2 = ee2.reverse_time - rj->reverse_time;
            double time_delta = time_delta1 + time_delta2;
            double load_delta1 = overload1 - ri->overload;
            double load_delta2 = overload2 - rj->overload;
            double load_delta = load_delta1 + load_delta2;
            double score = getScore(cost_delta, time_delta, load_delta);

            if (betterMove(res, score)) {
                res.type = "INTER_RELOCATE_" + std::to_string(len);
                res.empty = false;
                res.score = score;
                res.ri = ri;
                res.rj = rj;
                res.spi = SuperNode(first_i, last_i, len);
                res.pj = pj;
                if (LOG_LEVEL >= 1) {
                    res.cost_delta = cost_delta;
                    res.time_delta = time_delta;
                    res.load_delta = load_delta;
                    res.cost_value = cost_coef * cost_delta;
                    res.time_value = time_coef * time_delta * data->dist_time_ratio;
                    res.load_value = load_coef * data->std_dist_delta * load_delta / data->capacity;
                    res.cost_oi = ri->cost;
                    res.cost_oj = rj->cost;
                    res.time_oi = ri->reverse_time;
                    res.time_oj = rj->reverse_time;
                    res.load_oi = ri->overload;
                    res.load_oj = rj->overload;
                    res.cost_ni = ee1.cost;
                    res.cost_nj = ee2.cost;
                    res.time_ni = ee1.reverse_time;
                    res.time_nj = ee2.reverse_time;
                    res.load_ni = overload1;
                    res.load_nj = overload2;
                }
            }
        }
    }
}

void Operator::_interSwap(Solution *solution, Route *ri, Route *rj, int len_i, int len_j, EvalResult &res) {
    auto nodes_i = ri->nodes;
    auto nodes_j = rj->nodes;
    int size_i = nodes_i.size(), size_j = nodes_j.size();
    auto indices_i = random->randomInts(size_i);
    auto indices_j = random->randomInts(size_j);

    for (auto i : indices_i) {
        if (i >= (size_i + 1 - len_i)) continue;
        if (nodes_i[i]->id == 0) continue;
        for (auto j : indices_j) {
            if (j >= (size_j + 1 - len_j)) continue;
            if (nodes_j[j]->id == 0) continue;
            res.count++;
            Node *first_i = nodes_i[i], *last_i = nodes_i[i+len_i-1], *first_j = nodes_j[j], *last_j = nodes_j[j+len_j-1];
            Node *prev_i = first_i->prev, *prev_j = first_j->prev, *next_i = last_i->next, *next_j = last_j->next;
            if (skip(ri->id, prev_i->id, first_j->id) || skip(ri->id, last_j->id, next_i->id) ||
                skip(rj->id, prev_j->id, first_i->id) || skip(rj->id, last_i->id, next_j->id)) continue;
            double dist5 = data->distances[prev_i->id][first_j->id], dist6 = data->distances[last_j->id][next_i->id], dist7 = data->distances[prev_j->id][first_i->id], dist8 = data->distances[last_i->id][next_j->id];
            double time_dist5 = data->time_distances[prev_i->id][first_j->id], time_dist6 = data->time_distances[last_j->id][next_i->id], time_dist7 = data->time_distances[prev_j->id][first_i->id], time_dist8 = data->time_distances[last_i->id][next_j->id];
            EvalElem eei1 = EvalElem(ri, 0, i-1), eei2 = EvalElem(ri, i, i+len_i-1), eei3 = EvalElem(ri, i+len_i, ri->size);
            EvalElem eej1 = EvalElem(rj, 0, j-1), eej2 = EvalElem(rj, j, j+len_j-1), eej3 = EvalElem(rj, j+len_j, rj->size);
            EvalElem ee1 = eei1.add(eej2, dist5, time_dist5).add(eei3, dist6, time_dist6);
            EvalElem ee2 = eej1.add(eei2, dist7, time_dist7).add(eej3, dist8, time_dist8);
            double overload1 = std::max(ee1.max_load - data->capacity, 0.0);
            double overload2 = std::max(ee2.max_load - data->capacity, 0.0);

            // filter infeasible
            bool feasible1 = ee1.reverse_time < PRECISION && overload1 < PRECISION;
            bool feasible2 = ee2.reverse_time < PRECISION && overload2 < PRECISION;

            double cost_delta1 = ee1.cost - ri->cost, cost_delta2 = ee2.cost - rj->cost;
            double cost_delta = cost_delta1 + cost_delta2;
            double time_delta1 = ee1.reverse_time - ri->reverse_time;
            double time_delta2 = ee2.reverse_time - rj->reverse_time;
            double time_delta = time_delta1 + time_delta2;
            double load_delta1 = overload1 - ri->overload;
            double load_delta2 = overload2 - rj->overload;
            double load_delta = load_delta1 + load_delta2;
            double score = getScore(cost_delta, time_delta, load_delta);

            if (betterMove(res, score)) {
                res.type = "INTER_SWAP_" + std::to_string(len_i) + "_" + std::to_string(len_j);
                res.empty = false;
                res.score = score;
                res.ri = ri;
                res.rj = rj;
                res.spi = SuperNode(first_i, last_i, len_i);
                res.spj = SuperNode(first_j, last_j, len_j);
                if (LOG_LEVEL >= 1) {
                    res.cost_delta = cost_delta;
                    res.time_delta = time_delta;
                    res.load_delta = load_delta;
                    res.cost_value = cost_coef * cost_delta;
                    res.time_value = time_coef * time_delta * data->dist_time_ratio;
                    res.load_value = load_coef * data->std_dist_delta * load_delta / data->capacity;
                    res.cost_oi = ri->cost;
                    res.cost_oj = rj->cost;
                    res.time_oi = ri->reverse_time;
                    res.time_oj = rj->reverse_time;
                    res.load_oi = ri->overload;
                    res.load_oj = rj->overload;
                    res.cost_ni = ee1.cost;
                    res.cost_nj = ee2.cost;
                    res.time_ni = ee1.reverse_time;
                    res.time_nj = ee2.reverse_time;
                    res.load_ni = overload1;
                    res.load_nj = overload2;
                }
            }
        }
    }
}

void Operator::_twoOptStar(Solution *solution, Route *ri, Route *rj, EvalResult &res) {
    auto nodes_i = ri->nodes;
    auto nodes_j = rj->nodes;
    int size_i = nodes_i.size(), size_j = nodes_j.size();
    auto indices_i = random->randomInts(size_i);
    auto indices_j = random->randomInts(size_j);
    for (auto i : indices_i) {
        for (auto j : indices_j) {
            if (nodes_i[i]->id == 0 && nodes_j[j]->id == 0) continue;
            if (nodes_i[i]->next->id == 0 && nodes_j[j]->next->id == 0) continue;
            Node *pi = nodes_i[i], *pj = nodes_j[j];
            res.count++;
            if (skip(ri->id, pi->id, pj->next->id) || skip(rj->id, pj->id, pi->next->id)) continue;
            double dist3 = data->distances[pi->id][pj->next->id], dist4 = data->distances[pj->id][pi->next->id];
            double time_dist3 = data->time_distances[pi->id][pj->next->id], time_dist4 = data->time_distances[pj->id][pi->next->id];
            // (0, i) (i+1, end) (0, j) (j+1, end)
            EvalElem eei1 = EvalElem(ri, 0, i), eei2 = EvalElem(ri, i+1, ri->size);
            EvalElem eej1 = EvalElem(rj, 0, j), eej2 = EvalElem(rj, j+1, rj->size);
            EvalElem ee1 = eei1.add(eej2, dist3, time_dist3), ee2 = eej1.add(eei2, dist4, time_dist4);
            double overload1 = std::max(ee1.max_load - data->capacity, 0.0);
            double overload2 = std::max(ee2.max_load - data->capacity, 0.0);
            // filter infeasible
            bool feasible1 = ee1.reverse_time < PRECISION && overload1 < PRECISION;
            bool feasible2 = ee2.reverse_time < PRECISION && overload2 < PRECISION;

            double cost_delta = ee1.cost + ee2.cost - ri->cost - rj->cost;
            double time_delta1 = ee1.reverse_time - ri->reverse_time;
            double time_delta2 = ee2.reverse_time - rj->reverse_time;
            double time_delta = time_delta1 + time_delta2;
            double load_delta1 = overload1 - ri->overload;
            double load_delta2 = overload2 - rj->overload;
            double load_delta = load_delta1 + load_delta2;
            double score = getScore(cost_delta, time_delta, load_delta);

            if (betterMove(res, score)) {
                res.type = "TWO_OPT_STAR";
                res.empty = false;
                res.score = score;
                res.ri = ri;
                res.rj = rj;
                res.pi = pi;
                res.pj = pj;
                if (LOG_LEVEL >= 1) {
                    res.cost_delta = cost_delta;
                    res.time_delta = time_delta;
                    res.load_delta = load_delta;
                    res.cost_value = cost_coef * cost_delta;
                    res.time_value = time_coef * time_delta * data->dist_time_ratio;
                    res.load_value = load_coef * data->std_dist_delta * load_delta / data->capacity;
                    res.cost_oi = ri->cost;
                    res.cost_oj = rj->cost;
                    res.time_oi = ri->reverse_time;
                    res.time_oj = rj->reverse_time;
                    res.load_oi = ri->overload;
                    res.load_oj = rj->overload;
                    res.cost_ni = ee1.cost;
                    res.cost_nj = ee2.cost;
                    res.time_ni = ee1.reverse_time;
                    res.time_nj = ee2.reverse_time;
                    res.load_ni = overload1;
                    res.load_nj = overload2;
                }
            }
        }
    }
}

EvalResult Operator::relocate(Solution *solution, int len) {
    EvalResult res;
    int r_size = solution->routes.size();
    std::vector<int> rids = random->randomInts(r_size);
    // inter relocate
    if (r_size >= 2) {
        for (auto &i: rids) {
            for (auto &j: rids) {
                if (i == j) continue;
                _interRelocate(solution, solution->routes[i], solution->routes[j], len, res);
                if (res.done) break;
            }
            if (res.done) break;
        }
    }
    // intra relocate
    if (!res.done) {
        for (auto &i: rids) {
            _intraRelocate(solution, solution->routes[i], len, res);
        }
    }
    if (acceptMove(res)) {
        solution->relocate(res.ri, res.rj, res.spi, res.pj);
        res.accepted = true;
    }
    return res;
}

EvalResult Operator::swap(Solution *solution, int len_i, int len_j) {
    EvalResult res;
    int r_size = solution->routes.size();
    auto rids = random->randomInts(r_size);
    // inter swap
    if (r_size >= 2) {
        for (auto &i: rids) {
            for (auto &j: rids) {
                if (i == j || len_i == len_j && i > j) continue;
                _interSwap(solution, solution->routes[i], solution->routes[j], len_i, len_j, res);
                if (res.done) break;
            }
            if (res.done) break;
        }
    }

    // intra swap
    if (!res.done) {
        for (auto &i: rids) {
            _intraSwap(solution, solution->routes[i], len_i, len_j, res);
            if (res.done) break;
            if (len_i != len_j) {
                _intraSwap(solution, solution->routes[i], len_j, len_i, res);
                if (res.done) break;
            }
        }
    }

    if (acceptMove(res)) {
        solution->swap(res.ri, res.rj, res.spi, res.spj);
        res.accepted = true;
    }
    return res;
}

EvalResult Operator::twoOptStar(Solution *solution) {
    EvalResult res;
    int r_size = solution->routes.size();
    if (r_size < 2) return res;
    auto rids = random->randomInts(r_size);

    for (auto &i : rids) {
        for (auto &j : rids) {
            if (i <= j) continue;
            _twoOptStar(solution, solution->routes[i], solution->routes[j], res);
            if (res.done) break;
        }
        if (res.done) break;
    }
    
    if (acceptMove(res)) {
        solution->twoOptStar(res.ri, res.rj, res.pi, res.pj);
        res.accepted = true;
    }
    return res;
}

EvalResult Operator::operate(Solution *solution, int operate) {
    EvalResult res;
    switch (operate) {
        case RELOCATE_1:
            res = relocate(solution, 1);
            break;
        case RELOCATE_2:
            res = relocate(solution, 2);
            break;
        case RELOCATE_3:
            res = relocate(solution, 3);
            break;
        case SWAP_1_1:
            res = swap(solution, 1, 1);
            break;
        case SWAP_1_2:
            res = swap(solution, 1, 2);
            break;
        case SWAP_2_2:
            res = swap(solution, 2, 2);
            break;
        case SWAP_1_3:
            res = swap(solution, 1, 3);
            break;
        case SWAP_2_3:
            res = swap(solution, 2, 3);
            break;
        case SWAP_3_3:
            res = swap(solution, 3, 3);
            break;
        case TWO_OPT_STAR:
            res = twoOptStar(solution);
            break;
        default:
            break;
    }
    return res;
}