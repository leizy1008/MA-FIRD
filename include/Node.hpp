/**
 * Node.hpp
 * created on : August 2023
 * author : Z.LEI
 **/

#ifndef MA_FIRD_NODE_HPP
#define MA_FIRD_NODE_HPP

#include <iostream>
#include <cmath>
#include <vector>
#include "Config.hpp"

class DataNode {
public:
    int id = 0;
    double x = 0, y = 0, delivery = 0, pickup = 0, open_time = 0, close_time = 0, service_time = 0;
    double min_departure_time = 0, max_departure_time = 0;
    DataNode(int id, double x, double y, double delivery, double pickup, double open_time, double close_time, double service_time);
    DataNode() = default;
    ~DataNode() = default;
};

class Node : public DataNode {
public:
    double arrive_time = 0, wait_time = 0, start_time = 0, departure_time = 0;
    double load = 0, reverse_time = 0;
    Node *prev = nullptr, *next = nullptr;
    bool tw_feasible = true, cap_feasible = true;
    Node(const DataNode &node);
    Node(const Node &node);
    Node() = default;
    ~Node() = default;
};

class SuperNode {
public:
    int len = 0;
    Node *first = nullptr, *last = nullptr, *prev = nullptr, *next = nullptr;
    SuperNode() = default;
    SuperNode(Node *first, Node *last, int len) {
        this->first = first;
        this->last = last;
        this->prev = first->prev;
        this->next = last->next;
        this->len = len;
    }
};

#endif //MA_FIRD_NODE_HPP
