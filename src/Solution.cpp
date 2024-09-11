/**
 * Solution.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Solution.hpp"

Random *Solution::random = nullptr;
Parameters *Solution::params = nullptr;
Data *Solution::data = nullptr;

Solution::~Solution() {
    for (auto &route: routes) {
        delete route;
    }
    routes.clear();
}

Solution::Solution(const Solution &solution) {
    this->cost = solution.cost;
    this->total_cost = solution.total_cost;
    this->nb_vehicle = solution.nb_vehicle;
    this->tw_feasible = solution.tw_feasible;
    this->cap_feasible = solution.cap_feasible;
    this->feasible = solution.feasible;
    this->routes.resize(solution.routes.size(), nullptr);
    for (int i = 0; i < solution.routes.size(); ++i) {
        Route *route = new Route(*solution.routes[i]);
        this->routes[i] = route;
    }
}

void Solution::setContext(Random *_random, Parameters *_params, Data *_data) {
    Solution::random = _random;
    Solution::params = _params;
    Solution::data = _data;
    Insertion::setContext(_random, _params, _data);
}

void Solution::init() {
    RCRS();
}

void Solution::insertion(std::vector<Node *> &nodes, int opt) {
    insertor.run(routes, nodes, opt);
    update();
}

void Solution::RCRS() {
    // coefficients for terms
    double w1 = random->randomDouble(0, 1), w2 = random->randomDouble(0, 1);
    // candidate nodes
    std::vector<Node*> nodes;
    for (auto &n : data->nodes) {
        if (n.id == 0) continue;
        nodes.emplace_back(new Node(n));
    }

    // current route
    auto route = new Route(data->depot);
    route->update();
    this->routes.emplace_back(route);

    while (!nodes.empty()) {
        double best_score = INF;
        Node *best_node = nullptr, *best_pos = nullptr;
        double unrouted_sum_delivery = 0, unrouted_sum_pickup = 0;
        for (auto &node: nodes) {
            unrouted_sum_delivery += node->delivery;
            unrouted_sum_pickup += node->pickup;
        }
        auto route_nodes = route->nodes;
        for (int i = 0; i < nodes.size(); ++i) {
            Node *node = nodes[i];
            double unrouted_delivery = unrouted_sum_delivery - node->delivery;
            double unrouted_pickup = unrouted_sum_pickup - node->pickup;
            Node *p = route->head;
            for (int j = 0; j < route->size; ++j) {
                double dist1 = data->distances[p->id][p->next->id], dist2 = data->distances[p->id][node->id], dist3 = data->distances[node->id][p->next->id];
                double time_dist1 = data->time_distances[p->id][p->next->id], time_dist2 = data->time_distances[p->id][node->id], time_dist3 = data->time_distances[node->id][p->next->id];
                EvalElem ee1 = EvalElem(route, 0, j), ee2 = EvalElem(route, j+1, route->size), ee3 = EvalElem(node);
                EvalElem ee = ee1.add(ee3, dist2, time_dist2).add(ee2, dist3, time_dist3);
                // filter infeasible solutions
                if (!(ee.reverse_time < PRECISION && (ee.max_load - data->capacity) < PRECISION)) {
                    p = p->next;
                    continue;
                }
                // TD
                double cost_delta = ee.cost - route->cost;
                // calculation of TC
                // insert node into route_nodes
                auto tmp_route_nodes = std::vector<Node*>(route_nodes);
                tmp_route_nodes.insert(tmp_route_nodes.begin() + j + 1, node);
                std::vector<double> rd = std::vector<double>(route->size + 1, 0);
                std::vector<double> rp = std::vector<double>(route->size + 1, 0);
                std::vector<double> cd = std::vector<double>(route->size + 1, 0);
                std::vector<double> cp = std::vector<double>(route->size + 1, 0);
                std::vector<double> load = std::vector<double>(route->size + 1, 0);
                double delivery_sum = route->head->load + node->delivery;
                double pickup_sum = route->head->prev->load + node->pickup;

                // forward
                for (int l = 0; l < route->size + 1; ++l) {
                    if (l == 0) {
                        load[l] = delivery_sum;
                        rd[l] = data->capacity - load[l];
                        cd[l] = data->distances[tmp_route_nodes[l]->id][tmp_route_nodes[(l+1) % (tmp_route_nodes.size())]->id];
                    } else {
                        load[l] = delivery_sum - tmp_route_nodes[l]->delivery + tmp_route_nodes[l]->pickup;
                        rd[l] = data->capacity - std::max(load[l], load[l - 1]);
                        cd[l] = cd[l-1] + data->distances[tmp_route_nodes[l]->id][tmp_route_nodes[(l+1) % (tmp_route_nodes.size())]->id];
                    }
                }

                // backward
                for (int l = route->size; l >= 0; --l) {
                    if (l == route->size) {
                        rp[l] = data->capacity - load[l];
                        cp[l] = data->distances[tmp_route_nodes[l]->id][tmp_route_nodes[(l+1) % (tmp_route_nodes.size())]->id];
                    } else {
                        rp[l] = data->capacity - std::max(load[l], load[l + 1]);
                        cp[l] = cp[l+1] + data->distances[tmp_route_nodes[l]->id][tmp_route_nodes[(l+1) % (tmp_route_nodes.size())]->id];
                    }
                }

                // calculate rd_sum, cd_sum, rp_sum, cp_sum
                double rd_sum = 0, cd_sum = 0, rp_sum = 0, cp_sum = 0;
                for (int l = 0; l < route->size + 1; ++l) {
                    rd_sum += rd[l] * cd[l];
                    cd_sum += cd[l];
                    rp_sum += rp[l] * cp[l];
                    cp_sum += cp[l];
                }

                double rdt = rd_sum / cd_sum;
                double rpt = rp_sum / cp_sum;
                double tc = (unrouted_delivery / data->delivery_sum) * (1 - rdt / data->capacity) +
                            (unrouted_pickup / data->pickup_sum) * (1 - rpt / data->capacity);
                // RS
                double rs = data->distances[0][node->id] + data->distances[node->id][0];
                double score = cost_delta + w1 * tc * data->std_dist_delta - w2 * rs;
                if (score < best_score) {
                    best_score = score;
                    best_node = node;
                    best_pos = p;
                }
                p = p->next;
            }
        }
        if (best_pos == nullptr) {
            // creat new route
            route = new Route(data->depot);
            route->update();
            this->routes.emplace_back(route);
        } else {
            // insert node to route
            route->insert(best_pos, best_node);
            route->update();
            nodes.erase(std::remove(nodes.begin(), nodes.end(), best_node), nodes.end());
        }
    }
    // add ids
    for (int i = 0; i < this->routes.size(); ++i) {
        this->routes[i]->id = i + 1;
    }
    update();
}


void Solution::decreaseRoute() {
    // min route
    Route *selected_route = nullptr;
    for (auto &route: routes) {
        if (selected_route == nullptr || route->size < selected_route->size) {
            selected_route = route;
        }
    }
    routes.erase(std::remove(routes.begin(), routes.end(), selected_route), routes.end());
    std::vector<Node*> nodes;
    for (auto &node: selected_route->nodes) {
        if (node->id != 0) {
            nodes.emplace_back(new Node(*node));
        }
    }
    insertion(nodes, 3);
    delete selected_route;
}

std::vector<std::vector<int>> Solution::getMatrix() {
    std::vector<std::vector<int>> matrix(data->nodes.size(), std::vector<int>(data->nodes.size(), 0));
    for (auto &route: routes) {
        Node *p = route->head->next;
        while(p != route->head) {
            matrix[p->prev->id][p->id] = 1;
            p = p->next;
        }
        matrix[p->prev->id][p->id] = 1;
    }
    return matrix;
}

void Solution::update() {
    // delete empty routes
    for (auto it = routes.begin(); it != routes.end();) {
        if ((*it)->removed) {
            delete *it;
            it = routes.erase(it);
        } else {
            ++it;
        }
    }

    nb_vehicle = routes.size();

    cost = 0;
    tw_feasible = true;
    cap_feasible = true;
    for (auto &route : this->routes) {
        tw_feasible &= route->tw_feasible;
        cap_feasible &= route->cap_feasible;
        cost += route->cost;
    }
    feasible = tw_feasible && cap_feasible;

    total_cost = cost + nb_vehicle * data->dispatch_cost;
}

void Solution::print() {
    if (this == nullptr) {
        std::cout << "Solution: null" << std::endl;
        return;
    }
    if (LOG_LEVEL == 1) {
        // summary
        std::cout << "Solution: " << nb_vehicle << " " << cost << " " << total_cost << " (" << (tw_feasible ? 1 : 0) << ")(" << (cap_feasible ? 1 : 0) << ") ";
        std::cout << std::endl;
    } else if (LOG_LEVEL >= 2) {
        // details
        std::cout << "Solution: " << nb_vehicle << " " << cost << " " << total_cost << " [" << (tw_feasible ? 1 : 0) << "][" << (cap_feasible ? 1 : 0) << "] " << std::endl;
        for (auto &route: routes) {
            route->print();
        }
        std::cout << std::endl;
    }
}


double Solution::distance(Solution *other) {
    double similarity = 0, sa_edge_size = 0, sb_edge_size = 0;
    auto matrix1 = this->getMatrix();
    auto matrix2 = other->getMatrix();
    for (int i = 0; i < matrix1.size(); ++i) {
        for (int j = 0; j < matrix2.size(); ++j) {
            similarity += matrix1[i][j] * matrix2[i][j];
            sa_edge_size += matrix1[i][j];
            sb_edge_size += matrix2[i][j];
        }
    }
    double edge_size = std::max(sa_edge_size, sb_edge_size);
    double distance = 100 * (1 - similarity / edge_size);
    return distance;
}


bool Solution::betterThan(Solution *solution) const {
    if (solution == nullptr) return true;
    if (!this->feasible) return false;
    else if (!solution->feasible) return true;
    // both are feasible
    return this->total_cost < solution->total_cost - PRECISION;
}

void Solution::write(int iter) {
    auto &output_dir = params->output_dir;
    auto &filename = params->filename;
    auto &timestamp = params->timestamp;
    try {
        if (! std::filesystem::exists(output_dir)) {
            std::filesystem::create_directory(output_dir);
        }
        std::string res_dir = output_dir + filename + "-" + timestamp + "/";
        if (! std::filesystem::exists(res_dir)) {
            std::filesystem::create_directory(res_dir);
        }
        std::ofstream out;
        std::string res_file = filename + "-" + timestamp + "-"+ std::to_string(iter);
        out.open(res_dir + res_file +  ".txt");
        out << "NV: " << nb_vehicle << " TD: " << cost << " TC: " << total_cost << std::endl;
        // write routes
        int route_id = 1;
        for (auto &route: this->routes) {
            out << "Route " << route_id++ << ": ";
            Node* p = route->head->next;
            while (p != route->head) {
                out << p->id << " ";
                p = p->next;
            }
            out << std::endl;
        }
        out.close();
    } catch (std::exception &e) {
        std::cerr << "[ERROR] write solution error" << std::endl;
    }
}

void Solution::relocate(Route* &r1, Route* &r2, SuperNode &node1, Node *node2) {
    bool flag1 = false, flag2 = false;
    if (r2) {
        // inter relocate
        r1->remove(node1);
        r2->insert(node2, node2->next, node1);
        r1->update();
        r2->update();
        if (r1->removed) {
            flag1 = true;
        }
        if (r2->removed) {
            flag2 = true;
        }
    } else {
        // intra relocate
        r1->remove(node1);
        r1->insert(node2, node2->next, node1);
        r1->update();
        if (r1->removed) {
            flag1 = true;
        }
    }
    update();
    if (flag1) {
        r1 = nullptr;
    }
    if (flag2) {
        r2 = nullptr;
    }
}

void Solution::swap(Route* &r1, Route* &r2, SuperNode &node1, SuperNode &node2) {
    bool flag1 = false, flag2 = false;
    Node *prev1 = node1.prev, *prev2 = node2.prev;
    Node *next1 = node1.next, *next2 = node2.next;
    if (r2) {
        // inter swap
        r1->remove(node1);
        r2->remove(node2);
        r1->insert(prev1, next1, node2);
        r2->insert(prev2, next2, node1);
        r1->update();
        r2->update();
        if (r1->removed) {
            flag1 = true;
        }
        if (r2->removed) {
            flag2 = true;
        }
    } else {
        // intra swap
        r1->remove(node1);
        r1->remove(node2);
        r1->insert(prev1, next1, node2);
        r1->insert(prev2, next2, node1);
        r1->update();
        if (r1->removed) {
            flag1 = true;
        }
    }
    update();
    if (flag1) {
        r1 = nullptr;
    }
    if (flag2) {
        r2 = nullptr;
    }
}

void Solution::twoOptStar(Route* &r1, Route* &r2, Node *node1, Node *node2) {
    bool flag1 = false, flag2 = false;
    Node* next1 = node1->next, *next2 = node2->next;
    Node* last1 = r1->head->prev, *last2 = r2->head->prev;
    if (node1 == last1) {
        node2->next = r2->head;
        r2->head->prev = node2;
    } else {
        node2->next = next1;
        next1->prev = node2;
        last1->next = r2->head;
        r2->head->prev = last1;
    }
    if (node2 == last2) {
        node1->next = r1->head;
        r1->head->prev = node1;
    } else {
        node1->next = next2;
        next2->prev = node1;
        last2->next = r1->head;
        r1->head->prev = last2;
    }
    r1->update();
    r2->update();
    if (r1->removed) {
        flag1 = true;
    }
    if (r2->removed) {
        flag2 = true;
    }
    update();
    if (flag1) {
        r1 = nullptr;
    }
    if (flag2) {
        r2 = nullptr;
    }
}