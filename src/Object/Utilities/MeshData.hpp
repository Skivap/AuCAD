#ifndef MESH_DATA_HPP
#define MESH_DATA_HPP

#include <Eigen/Dense>
#include <glad/glad.h>

class Vertex;
class Edge;
class Triangle;
class HalfEdge;

// MeshData ======================================================================================
class MeshData{
private:
    std::vector<HalfEdge> m_halfEdges;
    std::vector<Triangle> m_triangles;
    std::vector<Edge> m_edges;
    std::vector<Vertex> m_vertices;

    GLuint m_VBOmesh, m_VBOwireframe;

public:
    // The VBO assignment is assume that we store the vertices without EBO, and the VAO has the same order with indices
    // Yes, there will be a duplicate vertices inside VBO
    MeshData(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals,
             const std::vector<Eigen::Vector3i>& indices);

    void assignVBO(GLuint VBOmesh, GLuint VBOwireframe);

    const std::vector<HalfEdge>& getHalfEdges() { return m_halfEdges; }
    const std::vector<Triangle>& getTriangles() { return m_triangles; }
    const std::vector<Edge>& getEdges() { return m_edges; }
    const std::vector<Vertex>& getVertices() { return m_vertices; }

private:
    Eigen::Vector3f m_meshColor, m_wireframeColor, m_pointsColor;
    Eigen::Vector3f m_meshSelectColor, m_wireframeSelectColor, m_pointsSelectColor;

    std::vector<bool> m_selectedVertices;
    std::vector<bool> m_selectedEdges;
    std::vector<bool> m_selectedTriangles;

public:
    void resetSelection();
    void selectTriangle(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& point);
};

// Vertex Data ======================================================================================
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

// Edge Data ======================================================================================
class Edge {
public:
    unsigned int index;

    HalfEdge* he;

public:
    Edge() :
        he(nullptr) {}
    ~Edge() {}

};

// Triangle Data ======================================================================================
class Triangle {
public:
    unsigned int index;

    HalfEdge* he;

public:
    Triangle() :
        he(nullptr) {}
    ~Triangle() {}
};

// Half Edge Data ======================================================================================
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
