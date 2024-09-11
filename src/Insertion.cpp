/**
 * Insertion.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Insertion.hpp"

Random *Insertion::random = nullptr;
Parameters *Insertion::params = nullptr;
Data *Insertion::data = nullptr;

void Insertion::setContext(Random *_random, Parameters *_params, Data *_data) {
    Insertion::random = _random;
    Insertion::params = _params;
    Insertion::data = _data;
}

void Insertion::run(std::vector<Route*> &routes, std::vector<Node*> &nodes, int opt) {
    switch (opt) {
        case 0:
            feasibleBestInsert(routes, nodes);
            break;
        case 1:
            feasibleRegretInsert(routes, nodes);
            break;
        case 2:
            infeasibleBestInsert(routes, nodes);
            break;
        case 3:
            infeasibleRegretInsert(routes, nodes);
            break;
        case 4:
            randomInsert(routes, nodes);
            break;
        default:
            std::cerr << "[ERROR] invalid insertion operator" << std::endl;
            exit(1);
    }
}


void Insertion::randomInsert(std::vector<Route*> &routes, std::vector<Node *> &nodes) {
    while (!nodes.empty()) {
        int route_id = random->randomInt(routes.size());
        int pos = random->randomInt(routes[route_id]->size);
        auto pos_node = routes[route_id]->nodes[pos];
        Node *node = nodes.back();
        nodes.pop_back();
        routes[route_id]->insert(pos_node, node);
        routes[route_id]->update();
    }
}

void Insertion::feasibleBestInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        double best_score = INF;
        Route *best_route = nullptr;
        Node *best_pos = nullptr;
        Node *node = nodes[i];
        for (int j = 0; j < routes.size(); ++j) {
            Node *p = routes[j]->head;
            for (int k = 0; k < routes[j]->size; ++k) {
                double dist1 = data->distances[p->id][p->next->id], dist2 = data->distances[p->id][node->id], dist3 = data->distances[node->id][p->next->id];
                double time_dist1 = data->time_distances[p->id][p->next->id], time_dist2 = data->time_distances[p->id][node->id], time_dist3 = data->time_distances[node->id][p->next->id];
                EvalElem ee1 = EvalElem(routes[j], 0, k), ee2 = EvalElem(routes[j], k+1, routes[j]->size), ee3 = EvalElem(node);
                EvalElem ee = ee1.add(ee3, dist2, time_dist2).add(ee2, dist3, time_dist3);
                double score = ee.cost - routes[j]->cost;
                if (ee.reverse_time < PRECISION && (ee.max_load - data->capacity) < PRECISION && score < best_score) {
                    best_score = score;
                    best_route = routes[j];
                    best_pos = p;
                }
                p = p->next;
            }
        }
        if (best_route == nullptr) {
            // creat new route
            best_route = new Route(data->depot);
            best_route->update();
            routes.emplace_back(best_route);
            best_pos = best_route->head;
        }
        best_route->insert(best_pos, node);
        best_route->update();
    }
    // update route ids
    for (int i = 0; i < routes.size(); ++i) {
        routes[i]->id = i + 1;
    }
}

void Insertion::infeasibleBestInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        double best_score = INF;
        Route *best_route = nullptr;
        Node *best_pos = nullptr;
        Node *node = nodes[i];
        for (int j = 0; j < routes.size(); ++j) {
            Node *p = routes[j]->head;
            for (int k = 0; k < routes[j]->size; ++k) {
                double dist1 = data->distances[p->id][p->next->id], dist2 = data->distances[p->id][node->id], dist3 = data->distances[node->id][p->next->id];
                double time_dist1 = data->time_distances[p->id][p->next->id], time_dist2 = data->time_distances[p->id][node->id], time_dist3 = data->time_distances[node->id][p->next->id];
                EvalElem ee1 = EvalElem(routes[j], 0, k), ee2 = EvalElem(routes[j], k+1, routes[j]->size), ee3 = EvalElem(node);
                EvalElem ee = ee1.add(ee3, dist2, time_dist2).add(ee2, dist3, time_dist3);
                double cost_delta = ee.cost - routes[j]->cost;
                double twp_delta = ee.reverse_time - routes[j]->reverse_time;
                double ccp_delta = std::max(ee.max_load - data->capacity, 0.0) - routes[j]->overload;
                double w1 = 1000, w2 = 1000;
                double score = cost_delta + w1 * twp_delta * data->dist_time_ratio + w2 * ccp_delta / data->capacity * data->std_dist_delta;
                if (score < best_score) {
                    best_score = score;
                    best_route = routes[j];
                    best_pos = p;
                }
                p = p->next;
            }
        }
        best_route->insert(best_pos, node);
        best_route->update();
    }
}

void Insertion::infeasibleRegretInsert(std::vector<Route*> &routes, std::vector<Node *> &nodes) {
    while (!nodes.empty()) {
        double best_regret = -INF;
        Route *best_route = nullptr;
        Node *best_node = nullptr, *best_pos = nullptr;
        for (int i = 0; i < nodes.size(); ++i) {
            Node *node = nodes[i];
            std::vector<std::tuple<int, Node*, double>> route_best_scores(routes.size(), std::make_tuple(0, nullptr, INF));
            for (int j = 0; j < routes.size(); ++j) {
                Node *p = routes[j]->head;
                for (int k = 0; k < routes[j]->size; ++k) {
                    double dist1 = data->distances[p->id][p->next->id], dist2 = data->distances[p->id][node->id], dist3 = data->distances[node->id][p->next->id];
                    double time_dist1 = data->time_distances[p->id][p->next->id], time_dist2 = data->time_distances[p->id][node->id], time_dist3 = data->time_distances[node->id][p->next->id];
                    EvalElem ee1 = EvalElem(routes[j], 0, k), ee2 = EvalElem(routes[j], k+1, routes[j]->size), ee3 = EvalElem(node);
                    EvalElem ee = ee1.add(ee3, dist2, time_dist2).add(ee2, dist3, time_dist3);
                    double cost_delta = ee.cost - routes[j]->cost;
                    double twp_delta = ee.reverse_time - routes[j]->reverse_time;
                    double ccp_delta = std::max(ee.max_load - data->capacity, 0.0) - routes[j]->overload;
                    double w1 = 1000, w2 = 1000;
                    double score = cost_delta + w1 * twp_delta * data->dist_time_ratio + w2 * ccp_delta / data->capacity * data->std_dist_delta;

                    if (score < std::get<2>(route_best_scores[j])) {
                        std::get<0>(route_best_scores[j]) = j;
                        std::get<1>(route_best_scores[j]) = p;
                        std::get<2>(route_best_scores[j]) = score;
                    }
                    p = p->next;
                }
            }
            std::sort(route_best_scores.begin(), route_best_scores.end(), [](std::tuple<int, Node*, double> &a, std::tuple<int, Node*, double> &b) {
                return std::get<2>(a) < std::get<2>(b);
            });
            double regret = -std::get<2>(route_best_scores[0]);
            if (route_best_scores.size() >= 2) {
                regret = std::get<2>(route_best_scores[1]) - std::get<2>(route_best_scores[0]);
            }
            if (regret > best_regret) {
                best_regret = regret;
                best_node = node;
                best_route = routes[std::get<0>(route_best_scores[0])];
                best_pos = std::get<1>(route_best_scores[0]);
            }
        }
        best_route->insert(best_pos, best_node);
        best_route->update();
        nodes.erase(std::remove(nodes.begin(), nodes.end(), best_node), nodes.end());
    }
}

void Insertion::feasibleRegretInsert(std::vector<Route*> &routes, std::vector<Node *> &nodes) {
    while (!nodes.empty()) {
        double best_regret = -INF;
        Route *best_route = nullptr;
        Node *best_node = nullptr, *best_pos = nullptr;
        for (int i = 0; i < nodes.size(); ++i) {
            Node *node = nodes[i];
            std::vector<std::tuple<int, Node*, double>> route_best_scores(routes.size() + 1, std::make_tuple(0, nullptr, INF));
            for (int j = 0; j < routes.size(); ++j) {
                Node *p = routes[j]->head;
                for (int k = 0; k < routes[j]->size; ++k) {
                    double dist1 = data->distances[p->id][p->next->id], dist2 = data->distances[p->id][node->id], dist3 = data->distances[node->id][p->next->id];
                    double time_dist1 = data->time_distances[p->id][p->next->id], time_dist2 = data->time_distances[p->id][node->id], time_dist3 = data->time_distances[node->id][p->next->id];
                    EvalElem ee1 = EvalElem(routes[j], 0, k), ee2 = EvalElem(routes[j], k+1, routes[j]->size), ee3 = EvalElem(node);
                    EvalElem ee = ee1.add(ee3, dist2, time_dist2).add(ee2, dist3, time_dist3);
                    double cost_delta = ee.cost - routes[j]->cost;
                    double score = cost_delta;
                    if (ee.reverse_time < PRECISION && (ee.max_load - data->capacity) < PRECISION && score < std::get<2>(route_best_scores[j])) {
                        std::get<0>(route_best_scores[j+1]) = j + 1;
                        std::get<1>(route_best_scores[j+1]) = p;
                        std::get<2>(route_best_scores[j+1]) = score;
                    }
                    p = p->next;
                }
            }
            // calculate the score for inserting into new route
            double score = data->distances[0][node->id] + data->distances[node->id][0];
            std::get<0>(route_best_scores[0]) = 0;
            std::get<1>(route_best_scores[0]) = nullptr;
            std::get<2>(route_best_scores[0]) = score;
            std::sort(route_best_scores.begin(), route_best_scores.end(), [](std::tuple<int, Node*, double> &a, std::tuple<int, Node*, double> &b) {
                return std::get<2>(a) < std::get<2>(b);
            });
            double regret = - std::get<2>(route_best_scores[0]);
            if (route_best_scores.size() >= 2) {
                regret = std::get<2>(route_best_scores[1]) - std::get<2>(route_best_scores[0]);
            }
            if (regret > best_regret) {
                best_regret = regret;
                best_node = node;
                if (std::get<0>(route_best_scores[0]) == 0) {
                    best_route = nullptr;
                    best_pos = nullptr;
                } else {
                    best_route = routes[std::get<0>(route_best_scores[0])-1];
                    best_pos = std::get<1>(route_best_scores[0]);
                }
            }
        }
        if (best_route == nullptr) {
            // creat new route
            best_route = new Route(data->depot);
            best_route->update();
            routes.emplace_back(best_route);
            best_pos = best_route->head;
        }
        best_route->insert(best_pos, best_node);
        best_route->update();
        nodes.erase(std::remove(nodes.begin(), nodes.end(), best_node), nodes.end());
    }
    // update route ids
    for (int i = 0; i < routes.size(); ++i) {
        routes[i]->id = i + 1;
    }
}
