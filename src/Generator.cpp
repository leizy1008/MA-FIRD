/**
 * Generator.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Generator.hpp"


Random *Generator::random = nullptr;
Parameters *Generator::params = nullptr;
Data *Generator::data = nullptr;

void Generator::setContext(Random *_random, Parameters *_params, Data *_data) {
    Generator::random = _random;
    Generator::params = _params;
    Generator::data = _data;
    RLAgent::setContext(_random, _params, _data);
}

void Generator::init() {
    if (params->population_size == 1) return;
    max_parents_num = std::min(params->population_size, 10);
    n_parents = random->randomInt(max_parents_num - 1) + 2;
    ratio = params->init_ratio;
    // init parent agent
    parent_agent.init(max_parents_num - 1, 1);
    // init insert agent
    auto num_insertion = 5;
    insert_opt = random->randomInt(num_insertion);
    insert_agent.init(num_insertion, 1);
}

void Generator::update(std::vector<Solution *> &parents, Solution *offspring) {
    double reward;
    if (!offspring->feasible) {
        reward = -2 * abs_sum_reward / step;
    } else {
        reward = parents[0]->total_cost - offspring->total_cost;
    }
    abs_sum_reward += abs(reward);
    step++;

    if (!offspring->feasible) {
        ratio = std::min(ratio * 1.02, 0.999);
    }
    // update parent agent
    parent_agent.updateQValues(0, n_parents - 2, 0, reward);
    n_parents = parent_agent.selectAction(0) + 2;
    // update insert agent
    insert_agent.updateQValues(0, insert_opt, 0, reward);
    insert_opt = insert_agent.selectAction(0);
}

std::vector<int> Generator::selectParents(int pop_size) {
    std::vector<int> indices;
    if (pop_size < 2) {
        std::cerr << "[ERROR] no enough individuals in population" << std::endl;
        exit(1);
    }
    indices = random->randomInts(pop_size);
    // keep the first n elems
    if (pop_size > n_parents) {
        indices.erase(indices.begin() + n_parents, indices.end());
    } else {
        n_parents = pop_size;
    }
    return indices;
}

Solution *Generator::breed(std::vector<Solution *> &parents) {
    Solution* offspring = ARIX(parents);
    return offspring;
}

Solution *Generator::ARIX(const std::vector<Solution *>& parents) {
    auto dominant = parents[0];
    auto solution = new Solution();
    auto route_size = dominant->routes.size();
    std::vector<bool> visited(data->nb_customers + 1, false);
    // routes pool with repeated count
    std::vector<std::pair<Route*, double>> dominant_routes, routes_pool;
    for (auto &route : dominant->routes) {
        dominant_routes.emplace_back(route, 0);
    }
    // create pool based on remaining parents
    for (int i = 1; i < n_parents; ++i) {
        for (auto &route : parents[i]->routes) {
            routes_pool.emplace_back(route, 0);
        }
    }
    int retained_route_size = std::max(floor(dominant->routes.size() * ratio), 1.0);
    int introduced_route_size = route_size - retained_route_size;
    if (introduced_route_size > routes_pool.size()) {
        introduced_route_size = routes_pool.size();
        retained_route_size = route_size - introduced_route_size;
    }
    if (retained_route_size--) {
        // select randomly the route
        auto route = new Route(*dominant_routes[random->randomInt(dominant_routes.size())].first);
        solution->routes.emplace_back(route);
        // remove route
        auto it = std::find_if(dominant_routes.begin(), dominant_routes.end(), [route](std::pair<Route*, double> a) {
            return a.first->cost == route->cost;
        });
        dominant_routes.erase(it);
        // update visited
        Node *p = route->head->next;
        while (p != route->head) {
            visited[p->id] = true;
            p = p->next;
        }
        // update repeated count
        for (auto &rt : routes_pool) {
            rt.second = 0;
            Node *q = rt.first->head->next;
            while (q != rt.first->head) {
                if (visited[q->id]) {
                    rt.second += 1;
                }
                q = q->next;
            }
        }
    }

    while (solution->routes.size() < route_size) {
        if (introduced_route_size-- > 0) {
            // calculate the conflict ratio (repeated number / length) for routes in routes_pool
            for (auto &route : routes_pool) {
                route.second = route.second / (route.first->nodes.size() - 1);
            }
            // order the routes by the conflict ratio
            std::sort(routes_pool.begin(), routes_pool.end(), [](std::pair<Route*, double> a, std::pair<Route*, double> b) {
                return a.second < b.second;
            });
            auto least_repeated = routes_pool[0].second;
            // the routes with the least repeated number
            std::vector<Route*> routes;
            for (auto &route : routes_pool) {
                if (route.second == least_repeated) {
                    routes.emplace_back(route.first);
                } else if (route.second > least_repeated) {
                    break;
                }
            }
            // select randomly the route from the least repeated routes
            auto route = new Route(*routes[random->randomInt(routes.size())]);
            solution->routes.emplace_back(route);
            // remove the selected route from the pool
            auto it = std::find_if(routes_pool.begin(), routes_pool.end(), [route](std::pair<Route*, double> a) {
                return a.first->cost == route->cost;
            });
            routes_pool.erase(it);
            // update visited
            Node *p = route->head->next;
            while (p != route->head) {
                visited[p->id] = true;
                p = p->next;
            }
            // update repeated count
            for (auto &rt : dominant_routes) {
                rt.second = 0;
                Node *q = rt.first->head->next;
                while (q != rt.first->head) {
                    if (visited[q->id]) {
                        rt.second += 1;
                    }
                    q = q->next;
                }
            }
            for (auto &rt : routes_pool) {
                rt.second = 0;
                Node *q = rt.first->head->next;
                while (q != rt.first->head) {
                    if (visited[q->id]) {
                        rt.second += 1;
                    }
                    q = q->next;
                }
            }
        }

        if (retained_route_size-- > 0) {
            // calculate the conflict ratio (repeated number / length) for routes in routes_pool
            for (auto &route : dominant_routes) {
                route.second = route.second / (route.first->nodes.size() - 1);
            }
            // order the routes by the conflict ratio
            std::sort(dominant_routes.begin(), dominant_routes.end(), [](std::pair<Route*, double> a, std::pair<Route*, double> b) {
                return a.second < b.second;
            });
            // the least repeated number
            auto least_repeated = dominant_routes[0].second;
            // the routes with the least repeated number
            std::vector<Route*> routes;
            for (auto &route : dominant_routes) {
                if (route.second == least_repeated) {
                    routes.emplace_back(route.first);
                } else if (route.second > least_repeated) {
                    break;
                }
            }
            // select randomly the route from the least repeated routes
            auto route = new Route(*routes[random->randomInt(routes.size())]);
            solution->routes.emplace_back(route);
            // remove route from dominant_routes
            auto it = std::find_if(dominant_routes.begin(), dominant_routes.end(), [route](std::pair<Route*, double> a) {
                return a.first->cost == route->cost;
            });
            dominant_routes.erase(it);
            // update visited
            Node *p = route->head->next;
            while (p != route->head) {
                visited[p->id] = true;
                p = p->next;
            }
            // update repeated count
            for (auto &rt : routes_pool) {
                rt.second = 0;
                Node *q = rt.first->head->next;
                while (q != rt.first->head) {
                    if (visited[q->id]) {
                        rt.second += 1;
                    }
                    q = q->next;
                }
            }
        }
    }
    // remove duplicate nodes
    visited.assign(data->nb_customers + 1, false);
    for (auto &route : solution->routes) {
        Node *p = route->head->next;
        while (p != route->head) {
            Node *q = nullptr;
            if (visited[p->id]) {
                route->remove(p);
                q = p;
            } else {
                visited[p->id] = true;
            }
            p = p->next;
            if (q != nullptr) {
                delete q;
            }
        }
        route->update();
    }
    // insert remaining nodes
    std::vector<Node*> nodes;
    for (int i = 1; i <= data->nb_customers; i++) {
        if (!visited[i]) {
            nodes.emplace_back(new Node(data->nodes[i]));
        }
    }
    random->shuffle(nodes);
    solution->insertion(nodes, insert_opt);

    // update ratio
    if (solution->nb_vehicle > dominant->nb_vehicle) {
        ratio = std::min(ratio * 1.01, 0.999);
    } else if (solution->nb_vehicle == dominant->nb_vehicle) {
        ratio = std::max(ratio * 0.99, 0.001);
    }

    return solution;
}