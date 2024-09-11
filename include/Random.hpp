/**
 * Random.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_RANDOM_HPP
#define MA_FIRD_RANDOM_HPP

#include <iostream>
#include <random>
#include <algorithm>
#include "Config.hpp"

class Random {
private:
    std::mt19937 generator;
public:
    Random() = default;
    ~Random() = default;
    Random(int seed) {
        srand(seed);
        generator.seed(seed);
    }

    // get random int
    int randomInt(int n) {
        if (n <= 0) throw std::invalid_argument("[ERROR] n must be positive");
        return rand() % n;
    }

    // get random double in uniform distribution
    double randomDouble(double a, double b) {
        std::uniform_real_distribution<double> distribution(a, b);
        return distribution(generator);
    }

    // get a permutation
    template <typename T>
    void shuffle(std::vector<T> &vec) {
        std::shuffle(vec.begin(), vec.end(), generator);
    }

    // get a random order ints of [0, n)
    std::vector<int> randomInts(int n) {
        std::vector<int> ints(n);
        std::iota(ints.begin(), ints.end(), 0);
        shuffle(ints);
        return ints;
    }
};


#endif //MA_FIRD_RANDOM_HPP
