/**
 * Population.cpp
 * created on : September 2023
 * author : Z.LEI
 **/

#include "Population.hpp"

Random *Population::random = nullptr;
Parameters *Population::params = nullptr;
Data *Population::data = nullptr;

Population::Population() = default;

Population::~Population() {
    for (auto &individual: population) {
        delete individual;
    }
    population.clear();
    delete best_solution;
}

void Population::setContext(Random *_random, Parameters *_params, Data *_data) {
    Population::random = _random;
    Population::params = _params;
    Population::data = _data;
    Searcher::setContext(_random, _params, _data);
    Generator::setContext(_random, _params, _data);
}

void Population::init() {
    generator.init();
    int max_trials = 5, trials = 0;
    while (population.size() < params->population_size && trials < max_trials) {
        auto solution = new Solution();
        solution->init();
        solution = searcher.FIRD(solution);
        if (add(solution)) {
            trials = 0;
        } else {
            trials++;
        }
    }
}

void Population::next() {
    if (population.empty() || population.size() == 1) {
        std::cerr << "[ERROR] there is no enough individuals in the population" << std::endl;
        exit(1);
    }
    Solution *solution;
    auto indices = generator.selectParents(population.size());
    std::vector<Solution*> parents;
    for (auto &index: indices) {
        parents.emplace_back(population[index]->solution);
    }
    solution = generator.breed(parents);
    solution = searcher.FIRD(solution);
    generator.update(parents, solution);

    if (add(solution) && population.size() >= 1.5 * params->population_size) {
        update();
    }

}

bool Population::add(Solution *solution) {
    std::vector<double> distances(population.size());
    double min_distance = INF;
    // calculate the distance with solutions in population
    for (int i = 0; i < population.size(); ++i) {
        double distance = solution->distance(population[i]->solution);
        distances[i] = distance;
        min_distance = std::min(min_distance, distance);
    }
    bool found_better = solution->betterThan(best_solution);
    if (found_better || solution->feasible && min_distance > PRECISION) {
        if (found_better) {
            delete best_solution;
            best_solution = new Solution(*solution);
        }
        // update distance in population
        for (int i = 0; i < population.size(); ++i) {
            population[i]->distance = std::min(population[i]->distance, distances[i]);
        }
        // add solution to population
        auto individual = new Individual(solution);
        individual->distance = min_distance;
        population.emplace_back(individual);
        return true;
    } else {
        delete solution;
        return false;
    }
}

void Population::update() {
    double max_cost, min_cost, max_distance, min_distance;
    double coef = params->fit_coef;
    // update the best solution
    std::sort(population.begin(), population.end(), [](Individual *a, Individual *b) {
        return a->solution->betterThan(b->solution);
    });
    max_cost = population[population.size() - 1]->solution->total_cost;
    min_cost = population[0]->solution->total_cost;
    // sort according to distance
    std::sort(population.begin(), population.end(), [](Individual *a, Individual *b) {
        return a->distance > b->distance;
    });
    max_distance = population[0]->distance;
    min_distance = population[population.size() - 1]->distance;
    // update the fitness
    for (int i = 0; i < population.size(); ++i) {
        population[i]->fitness = (max_cost - population[i]->solution->total_cost) / (max_cost - min_cost) +
                coef * (population[i]->distance - min_distance) / (max_distance - min_distance);
    }
    // sort the population according to the fitness
    std::sort(population.begin(), population.end(), [](Individual *a, Individual *b) {
        return a->fitness > b->fitness;
    });

    // delete the worst solutions
    while (population.size() > params->population_size) {
        delete population[population.size() - 1];
        population.pop_back();
    }
    updateDistances();
}

void Population::updateDistances() {
    double sum_distance = 0;
    std::vector<double> distances(population.size(), INF);
    for (int i = 0; i < population.size(); ++i) {
        for (int j = i + 1; j < population.size(); ++j) {
            double distance = population[i]->solution->distance(population[j]->solution);
            distances[i] = std::min(distances[i], distance);
            distances[j] = std::min(distances[j], distance);
            sum_distance += 2 * distance;
        }
    }
    for (int i = 0; i < population.size(); ++i) {
        population[i]->distance = distances[i];
    }
}

void Population::print() {
    std::cout << "population : " << std::endl;
    for (auto &ind: population) {
        ind->solution->print();
    }
    std::cout << std::endl;
}