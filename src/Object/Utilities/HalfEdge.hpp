#ifndef HALF_EDGE_HPP
#define HALF_EDGE_HPP

#include <Eigen/Dense>

class Vertex;
class Edge;
class Triangle;
class HalfEdge;

class Vertex {
public:
    HalfEdge* edge;

    Eigen::Vector3f pos;
    Eigen::Vector3f normal;
public:
    Vertex() :
        edge(nullptr) {}
    ~Vertex() {}
};

class Edge {
public:
    HalfEdge* edge;

public:
    Edge() :
        edge(nullptr) {}
    ~Edge() {}

};

class Triangle {
public:
    HalfEdge* edge;

public:
    Triangle() :
        edge(nullptr) {}
    ~Triangle() {}
};

class HalfEdge {
public:
    HalfEdge* next;
    HalfEdge* prev;
    HalfEdge* twin;

    Vertex* vertex;
    Edge* edge;
    Triangle* face;

public:
    HalfEdge() :
        next(nullptr), prev(nullptr), twin(nullptr),
        vertex(nullptr), edge(nullptr), face(nullptr) {}
    ~HalfEdge() {}

    inline void assign(HalfEdge* n, HalfEdge* p, HalfEdge* t) {
        next = n, prev = p, twin = t;
    }
    inline void assign(Vertex* v, Edge* e, Triangle* t) {
        vertex = v, edge = e, face = t;
    }
};

#endif // HALF_EDGE_HPP
