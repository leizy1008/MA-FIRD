/**
 * main.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include <iostream>
#include "Algorithm.hpp"

int main(int argc, char **argv) {
    std::cout << "Memetic Algorithm with Feasible and Infeasible Route Descent Search for VRPSPDTW" << std::endl << std::endl;
    auto params = new Parameters(argc, argv);
    params->print();
    Algorithm algo(params);
    algo.run();
    return 0;
}
