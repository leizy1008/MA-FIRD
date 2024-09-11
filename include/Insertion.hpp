/**
 * Insertion.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_INSERTION_HPP
#define MA_FIRD_INSERTION_HPP

#include <iostream>
#include <vector>
#include "Config.hpp"
#include "Random.hpp"
#include "Parameters.hpp"
#include "Data.hpp"
#include "Node.hpp"
#include "Route.hpp"
#include "Evaluation.hpp"

class Insertion {
private:
    static Random *random;
    static Parameters *params;
    static Data *data;
    void randomInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes);
    void feasibleBestInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes);
    void infeasibleBestInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes);
    void feasibleRegretInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes);
    void infeasibleRegretInsert(std::vector<Route*> &routes, std::vector<Node*> &nodes);
public:
    Insertion() = default;
    ~Insertion() = default;
    static void setContext(Random *_random, Parameters *_params, Data *_data);
    void run(std::vector<Route*> &routes, std::vector<Node*> &nodes, int opt);
};


#endif //MA_FIRD_INSERTION_HPP
