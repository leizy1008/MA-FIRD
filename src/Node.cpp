/**
 * Node.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Node.hpp"

DataNode::DataNode(int id, double x, double y, double delivery, double pickup, double open_time, double close_time, double service_time) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->delivery = delivery;
    this->pickup = pickup;
    this->open_time = open_time;
    this->close_time = close_time;
    this->service_time = service_time;
}


Node::Node(const DataNode &node) {
    this->id = node.id;
    this->x = node.x;
    this->y = node.y;
    this->delivery = node.delivery;
    this->pickup = node.pickup;
    this->open_time = node.open_time;
    this->close_time = node.close_time;
    this->service_time = node.service_time;
    this->min_departure_time = node.min_departure_time;
    this->max_departure_time = node.max_departure_time;
    this->arrive_time = 0;
    this->wait_time = 0;
    this->start_time = 0;
    this->departure_time = 0;
    this->prev = nullptr;
    this->next = nullptr;
    this->tw_feasible = true;
    this->cap_feasible = true;
    this->reverse_time = 0;
}

Node::Node(const Node &node) {
    this->id = node.id;
    this->x = node.x;
    this->y = node.y;
    this->delivery = node.delivery;
    this->pickup = node.pickup;
    this->open_time = node.open_time;
    this->close_time = node.close_time;
    this->service_time = node.service_time;
    this->min_departure_time = node.min_departure_time;
    this->max_departure_time = node.max_departure_time;
    this->arrive_time = node.arrive_time;
    this->wait_time = node.wait_time;
    this->start_time = node.start_time;
    this->departure_time = node.departure_time;
    this->prev = node.prev;
    this->next = node.next;
    this->tw_feasible = node.tw_feasible;
    this->cap_feasible = node.cap_feasible;
    this->load = node.load;
    this->reverse_time = node.reverse_time;
}