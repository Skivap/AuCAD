#ifndef MESH_DATA_HPP
#define MESH_DATA_HPP

#include <Eigen/Dense>
#include <glad/glad.h>

class Vertex;
class Edge;
class Triangle;
class HalfEdge;

class MeshData{
private:
    std::vector<HalfEdge> m_halfEdges;
    std::vector<Triangle> m_triangles;
    std::vector<Edge> m_edges;
    std::vector<Vertex> m_vertices;
    GLuint VBO;

public:
    // The VBO assignment is assume that we store the vertices without EBO, and the VAO has the same order with indices
    // Yes, there will be a duplicate vertices inside VBO
    MeshData(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals,
             const std::vector<Eigen::Vector3i>& indices, GLuint VBO);

    const std::vector<HalfEdge>& getHalfEdges() { return m_halfEdges; }
    const std::vector<Triangle>& getTriangles() { return m_triangles; }
    const std::vector<Edge>& getEdges() { return m_edges; }
    const std::vector<Vertex>& getVertices() { return m_vertices; }


};

class Vertex {
public:
    unsigned int index;

    HalfEdge* he;

    Eigen::Vector3f pos;
    Eigen::Vector3f normal;
public:
    Vertex() :
        he(nullptr) {}
    ~Vertex() {}
};

class Edge {
public:
    unsigned int index;

    HalfEdge* he;

public:
    Edge() :
        he(nullptr) {}
    ~Edge() {}

};

class Triangle {
public:
    unsigned int index;

    HalfEdge* he;

public:
    Triangle() :
        he(nullptr) {}
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

#endif // MESH_DATA_HPP
