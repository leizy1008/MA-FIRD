/**
 * Searcher.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Searcher.hpp"

Random *Searcher::random = nullptr;
Parameters *Searcher::params = nullptr;
Data *Searcher::data = nullptr;

Searcher::Searcher() {
    neighborhoods =  {
        RELOCATE_1,
        RELOCATE_2,
        RELOCATE_3,
        SWAP_1_1,
        SWAP_1_2,
        SWAP_2_2,
        SWAP_1_3,
        SWAP_2_3,
        SWAP_3_3,
        TWO_OPT_STAR,
    };
}

Searcher::~Searcher() {
    neighborhoods.clear();
}

void Searcher::setContext(Random *_random, Parameters *_params, Data *_data) {
    Searcher::random = _random;
    Searcher::data = _data;
    Searcher::params = _params;
    Operator::setContext(_random, _params, _data);
    RLAgent::setContext(_random, _params, _data);
}

Solution *Searcher::FIRD(Solution *solution) {
    auto best_solution = new Solution(*solution);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (!solution->feasible) {
        // Repair if the solution is not feasible
        solution = FIS(solution, best_solution);
        if (solution->betterThan(best_solution)) {
            delete best_solution;
            best_solution = new Solution(*solution);
        }
    }

    while (solution->feasible) {
        // Decrease the route and check feasibility
        solution->decreaseRoute();
        if (solution->nb_vehicle < data->min_vehicles) {
            solution->feasible = false;
            break;
        }

        if (solution->betterThan(best_solution)) {
            // Update the best solution if the current solution is better
            delete best_solution;
            best_solution = new Solution(*solution);
        } else {
            // Repair and update the best solution if the repaired solution is better
            solution = FIS(solution, best_solution);
            if (solution->betterThan(best_solution)) {
                delete best_solution;
                best_solution = new Solution(*solution);
            }
        }
    }

    if (best_solution->feasible) {
        best_solution = FIS(best_solution, nullptr);
    }

    delete solution;
    return best_solution;
}

Solution *Searcher::FIS(Solution *solution, Solution *objective) {
    opt.init(solution->feasible);
    auto *best_solution = new Solution(*solution);
    bool improved = true;
    int step = 0, stagnation = 0, patience = 500;
    while (improved && stagnation < patience && solution->nb_vehicle >= data->min_vehicles) {
        improved = false;
        auto actions = random->randomInts(neighborhoods.size());
        for (auto &action : actions) {
            EvalResult res = opt.operate(solution, neighborhoods[action]);
            improved = res.accepted;
            if (objective != nullptr && solution->betterThan(objective)) {
                delete best_solution;
                return solution;
            }
            if (solution->betterThan(best_solution)) {
                delete best_solution;
                best_solution = new Solution(*solution);
                stagnation = 0;
            } else {
                stagnation++;
            }
            step++;
            if (improved) break;
        }
        opt.update(solution);
    }

    delete solution;
    return best_solution;
}
