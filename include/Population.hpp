/**
 * Population.hpp
 * created on : September 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_POPULATION_HPP
#define MA_FIRD_POPULATION_HPP


#include "Config.hpp"
#include "Random.hpp"
#include "Parameters.hpp"
#include "Data.hpp"
#include "Solution.hpp"
#include "Searcher.hpp"
#include "Generator.hpp"

class Individual {
public:
    Solution *solution;
    double distance = 0;
    double fitness = 0;
    Individual(Solution *solution) {
        this->solution = solution;
    }
    ~Individual() {
        delete solution;
    }
};

class Population {
private:
    static Random *random;
    static Parameters *params;
    static Data *data;
    Searcher searcher;
    Generator generator;
    bool add(Solution *solution);
    void update();
    void updateDistances();
public:
    std::vector<Individual*> population {};
    Solution *best_solution = nullptr;
    Population();
    ~Population();
    static void setContext(Random *_random, Parameters *_params, Data *_data);
    void init();
    void next();
    void print();
};

#endif //MA_FIRD_POPULATION_HPP
