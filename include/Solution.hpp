/**
 * Solution.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_SOLUTION_HPP
#define MA_FIRD_SOLUTION_HPP

#include <vector>
#include <climits>
#include <filesystem>
#include "Random.hpp"
#include "Config.hpp"
#include "Node.hpp"
#include "Route.hpp"
#include "Data.hpp"
#include "Parameters.hpp"
#include "Evaluation.hpp"
#include "Insertion.hpp"

class Solution {
private:
    static Random *random;
    static Parameters *params;
    static Data *data;
    Insertion insertor;
public:
    double cost = 0;
    double total_cost = 0;
    int nb_vehicle = 0;
    bool tw_feasible = true;
    bool cap_feasible = true;
    bool feasible = true;
    std::vector<Route*> routes {};
    Solution(const Solution &solution);
    Solution() = default;
    ~Solution();
    static void setContext(Random *_random, Parameters *_params, Data *_data);
    void init();
    void update();
    double distance(Solution* other);
    bool betterThan(Solution *solution) const;
    void print();
    void write(int iter);
    std::vector<std::vector<int>> getMatrix();
    void decreaseRoute();
    void insertion(std::vector<Node*> &nodes, int opt);
    void RCRS();
    void relocate(Route* &r1, Route* &r2, SuperNode &node1, Node *node2);
    void swap(Route* &r1, Route* &r2, SuperNode &node1, SuperNode &node2);
    void twoOptStar(Route* &r1, Route* &r2, Node* node1, Node* node2);
};


#endif //MA_FIRD_SOLUTION_HPP
