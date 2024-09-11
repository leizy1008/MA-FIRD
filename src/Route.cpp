/**
 * Route.cpp
 * created on : August 2023
 * author : Z.LEI
 **/

#include "Route.hpp"

Random *Route::random = nullptr;
Parameters *Route::params = nullptr;
Data *Route::data = nullptr;

Route::Route(DataNode &node) {
    head = new Node(node);
    head->prev = head;
    head->next = head;
    size = 1;
    tw_feasible = true;
    cap_feasible = true;
    cost = 0;
    reverse_time = 0;
    max_load = 0;
    overload = 0;
}

Route::Route(const Route& route) {
    if (route.head == nullptr) return;
    head = new Node(*route.head);
    head->prev = head;
    head->next = head;
    id = route.id;
    size = route.size;
    removed = route.removed;
    tw_feasible = route.tw_feasible;
    cap_feasible = route.cap_feasible;
    cost = route.cost;
    reverse_time = route.reverse_time;
    max_load = route.max_load;
    overload = route.overload;
    cost_matrix = route.cost_matrix;
    in_load_matrix = route.in_load_matrix;
    out_load_matrix = route.out_load_matrix;
    max_load_matrix = route.max_load_matrix;
    open_time_matrix = route.open_time_matrix;
    close_time_matrix = route.close_time_matrix;
    duration_time_matrix = route.duration_time_matrix;
    wait_time_matrix = route.wait_time_matrix;
    reverse_time_matrix = route.reverse_time_matrix;
    nodes.emplace_back(head);
    Node *p = route.head->next;
    while (p != route.head) {
        auto q = new Node(*p);
        this->insert(q);
        nodes.emplace_back(q);
        p = p->next;
    }
}

Route::~Route() {
    if (head == nullptr) return;
    Node *p = head;
    Node *q = head->next;
    Node *last = head->prev;
    while (p != last) {
        delete p;
        p = q;
        q = q->next;
    }
    delete p;
    nodes.clear();
}

void Route::setContext(Random *_random, Parameters *_params, Data *_data) {
    Route::random = _random;
    Route::params = _params;
    Route::data = _data;
}

void Route::update() {
    size = 1;
    cost = 0;
    tw_feasible = true;
    cap_feasible = true;

    nodes.clear();
    nodes.emplace_back(head);
    Node *p = head->next;
    double delivery_sum = 0, pickup_sum = 0;
    while (p != head) {
        nodes.emplace_back(p);
        delivery_sum += p->delivery;
        pickup_sum += p->pickup;
        p = p->next;
    }

    // p = head
    p->load = delivery_sum;
    p->cap_feasible = p->load <= data->capacity;
    max_load = p->load;
    cap_feasible = cap_feasible && p->cap_feasible;
    reverse_time = 0;

    p = p->next;
    while (p != head) {
        p->arrive_time = p->prev->departure_time + data->time_distances[p->prev->id][p->id];
        if (p->arrive_time <= p->close_time) {
            p->wait_time = std::max(0.0, p->open_time - p->arrive_time);
            p->reverse_time = 0;
            p->tw_feasible = true;
        } else {
            p->wait_time = 0;
            p->reverse_time = p->arrive_time - p->close_time;
            p->tw_feasible = false;
        }
        p->start_time = p->arrive_time + p->wait_time - p->reverse_time;
        p->departure_time = p->start_time + p->service_time;
        p->load = p->prev->load - p->delivery + p->pickup;
        p->cap_feasible = p->load <= data->capacity;
        cost += data->distances[p->prev->id][p->id];
        reverse_time += p->reverse_time;
        max_load = std::max(max_load, p->load);
        cap_feasible = cap_feasible && p->cap_feasible;
        tw_feasible = tw_feasible && p->tw_feasible;
        size++;
        p = p->next;
    }
    // p = head
    p->arrive_time = p->prev->departure_time + data->time_distances[p->prev->id][p->id];
    p->wait_time = 0;
    if (p->arrive_time <= p->close_time) {
        p->tw_feasible = true;
        p->reverse_time = 0;
    } else {
        p->reverse_time = p->arrive_time - p->close_time;
        p->tw_feasible = false;
    }
    cost += data->distances[p->prev->id][p->id];
    reverse_time += p->reverse_time;
    tw_feasible = tw_feasible && p->tw_feasible;
    overload = std::max(0.0, max_load - data->capacity);
    removed = size <= 1;

    updateMatrix();
}


void Route::updateMatrix() {
    // clear all matrix
    cost_matrix.clear();
    in_load_matrix.clear();
    out_load_matrix.clear();
    max_load_matrix.clear();
    open_time_matrix.clear();
    close_time_matrix.clear();
    duration_time_matrix.clear();
    wait_time_matrix.clear();
    reverse_time_matrix.clear();
    // resize all matrix
    cost_matrix.resize(size + 1, std::vector<double>(size + 1));
    in_load_matrix.resize(size + 1, std::vector<double>(size + 1));
    out_load_matrix.resize(size + 1, std::vector<double>(size + 1));
    max_load_matrix.resize(size + 1, std::vector<double>(size + 1));
    open_time_matrix.resize(size + 1, std::vector<double>(size + 1));
    close_time_matrix.resize(size + 1, std::vector<double>(size + 1));
    duration_time_matrix.resize(size + 1, std::vector<double>(size + 1));
    wait_time_matrix.resize(size + 1, std::vector<double>(size + 1));
    reverse_time_matrix.resize(size + 1, std::vector<double>(size + 1));
    Node *p = head;
    for (int i = 0; i < size + 1; ++i) {
        Node *q = p;
        for (int j = i; j < size + 1; ++j) {
            if (i == j) {
                cost_matrix[i][j] = 0;
                in_load_matrix[i][j] = q->delivery;
                out_load_matrix[i][j] = q->pickup;
                max_load_matrix[i][j] = std::max(q->delivery, q->pickup);
                open_time_matrix[i][j] = q->open_time;
                close_time_matrix[i][j] = q->close_time;
                duration_time_matrix[i][j] = q->service_time;
                wait_time_matrix[i][j] = 0;
                reverse_time_matrix[i][j] = 0;
            }
            else {
                cost_matrix[i][j] = cost_matrix[i][j-1] + data->distances[q->prev->id][q->id];
                in_load_matrix[i][j] = in_load_matrix[i][j-1] + q->delivery;
                out_load_matrix[i][j] = out_load_matrix[i][j-1] + q->pickup;
                max_load_matrix[i][j] = std::max(max_load_matrix[i][j-1] + q->delivery, out_load_matrix[i][j-1] + std::max(q->delivery, q->pickup));
                double td = duration_time_matrix[i][j-1] + wait_time_matrix[i][j-1] + data->time_distances[q->prev->id][q->id] - reverse_time_matrix[i][j-1];
                double wt = std::max(q->open_time - (close_time_matrix[i][j-1] + td), 0.0);
                double rt = std::max(open_time_matrix[i][j-1] + td - q->close_time, 0.0);
                duration_time_matrix[i][j] = duration_time_matrix[i][j-1] + data->time_distances[q->prev->id][q->id] + q->service_time;
                open_time_matrix[i][j] = std::max(open_time_matrix[i][j-1], q->open_time - td) - wt;
                close_time_matrix[i][j] = std::min(close_time_matrix[i][j-1], q->close_time - td) + rt;
                wait_time_matrix[i][j] = wait_time_matrix[i][j-1] + wt;
                reverse_time_matrix[i][j] = reverse_time_matrix[i][j-1] + rt;
            }
            q = q->next;
        }
        p = p->next;
    }
}
void Route::print() const {
    Node* p = head;
    std::cout << cost << "(" << (this->tw_feasible ? 1 : 0) << ")(" << (this->cap_feasible ? 1 : 0) << "): ";
    while (p != head->prev) {
        std::cout << p->id << "(" << (p->tw_feasible ? 1 : 0) << ")(" << (p->cap_feasible ? 1 : 0) << "), ";
        p = p->next;
    }
    std::cout << p->id << "(" << (p->tw_feasible ? 1 : 0) << ")(" << (p->cap_feasible ? 1 : 0) << ")" << std::endl;
}

void Route::insert(Node *node) {
    if (head == nullptr) {
        head = node;
        head->prev = head;
        head->next = head;
    } else {
        Node* p = head->prev;
        p->next = node;
        head->prev = node;
        node->prev = p;
        node->next = head;
    }
}

void Route::insert(Node* pos, Node* node) {
    Node* next = pos->next;
    pos->next = node;
    next->prev = node;
    node->next = next;
    node->prev = pos;
}

void Route::insert(Node *prev, Node *next, SuperNode &node) {
    prev->next = node.first;
    node.first->prev = prev;
    next->prev = node.last;
    node.last->next = next;
}

void Route::remove(Node *pos) {
    if (head == nullptr) throw std::invalid_argument("[ERROR] Route is empty!");
    if (pos == head) throw std::invalid_argument("[ERROR] Cannot remove head node!");
    Node* prev = pos->prev;
    Node* next = pos->next;
    prev->next = next;
    next->prev = prev;
}

void Route::remove(SuperNode &node) {
    if (head == nullptr) throw std::invalid_argument("[ERROR] Route is empty!");
    if (node.first == head) throw std::invalid_argument("[ERROR] Cannot remove head node!");
    Node *prev = node.prev, *next = node.next;
    prev->next = next;
    next->prev = prev;
}
