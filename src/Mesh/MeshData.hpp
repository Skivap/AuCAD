#ifndef MESH_DATA_HPP
#define MESH_DATA_HPP

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <glad/glad.h>
#include <igl/arap.h>
#include <map>

#include "../Utilities/Shader.hpp"

#include "../Visualizer/Mesh.hpp"
#include "../Visualizer/Wireframe.hpp"
#include "../Visualizer/PointCloud.hpp"

// Include GenAPI for animation support
#include "../GenAPI/GenAPI.hpp"

class Vertex;
class Edge;
class Triangle;
class HalfEdge;

enum MeshVisMode{
    None        = 0x0,
    Normals     = 0x1,
    Weight      = 0x2
};

// MeshData ======================================================================================
class MeshData{
private:
    std::vector<HalfEdge> m_halfEdges;
    std::vector<Triangle> m_triangles;
    std::vector<Edge> m_edges;
    std::vector<Vertex> m_vertices;

    GLuint m_VBOmesh, m_VBOwireframe;

public:
    // Constructor =============================================================================================================
    // The VBO assignment is assume that we store the vertices without EBO, and the VAO has the same order with indices
    // Yes, there will be a duplicate vertices inside VBO

    MeshData(Shader* shader, Shader* wireframe_shader, Shader* pointcloud_shader, const std::string& filePath);
    void init(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals,
              const std::vector<Eigen::Vector3i>& indices);
    void initVisualizer(Shader* shader, Shader* wireframe_shader, Shader* pointcloud_shader,
   	                    const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals, const std::vector<Eigen::Vector3i>& indices);
    void draw(const CameraParam& cameraParam);

    const std::vector<HalfEdge>& getHalfEdges() { return m_halfEdges; }
    const std::vector<Triangle>& getTriangles() { return m_triangles; }
    const std::vector<Edge>& getEdges() { return m_edges; }
    const std::vector<Vertex>& getVertices() { return m_vertices; }

    Vertex& getVertex(int idx) { return m_vertices[idx]; }

    // Selection =============================================================================================================
private:
    Eigen::Vector3f m_meshColor, m_wireframeColor, m_pointsColor;
    Eigen::Vector3f m_meshSelectColor, m_wireframeSelectColor, m_pointsSelectColor;

    std::vector<bool> m_selectedVertices;
    std::vector<bool> m_selectedEdges;
    std::vector<bool> m_selectedTriangles;

    int lastSelectedVertex;

public:
    void resetSelection();
    void selectTriangle(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint);
    int selectVertex(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint);
    void deselectVertex(int index);
    void selectEdges(); // TODO

    const std::vector<bool>& getSelectedVertices(){ return m_selectedVertices; }

    static bool rayIntersectTriangle(const Eigen::Vector3f& org, const Eigen::Vector3f& dir, const Triangle& tri,
                                     Eigen::Vector3f& intersectPoint, float& t);

    const int getLastSelectedVertex() { return lastSelectedVertex; }

private:
    // Processor Implementation =============================================================================================================
    Eigen::MatrixXd m_V;   // Original positions
    Eigen::MatrixXi m_F;   // Face indices
    Eigen::VectorXi m_b;   // Constraint indices
    Eigen::MatrixXd m_bc;  // Constraint positions
    igl::ARAPData   m_arap_data;
public:
    void precomputeARAP();
    void precomputeConstraint();
    void computeARAP();
    void saveTimeFrame(float time);
    
    // Animation frame management
    void storeAnimationFrames(const GenAPI::AnimationSequence& frames);
    void applyAnimationFrame(int frameIndex);
    void clearAnimationFrames();
    bool hasAnimationFrames() const;
    int getAnimationFrameCount() const;

private:
    // Animation Implementation =============================================================================================================
    GenAPI::AnimationSequence m_storedAnimationFrames;
    std::map<int, Eigen::Vector3d> m_basePositions; // Store base positions before animation
    
    // Visualization Implementation =============================================================================================================
    Object::Mesh* m_mesh;
    Object::Wireframe* m_wireframe;
    Object::PointCloud* m_pointCloud;
public:
    void refreshTriangleColor(MeshVisMode mode, float scalar = 0.08f);
    void refreshEdgeColor();

    void changeTriangleColor(int idx, Eigen::Vector3f color);
    void changeVertexPosition(int idx, Eigen::Vector3f pos);

    void refreshPosition();
    void refreshPosition(float time);
};

// Vertex Data ======================================================================================
class Vertex {
public:
    unsigned int index;
    Eigen::Vector3d meanCurvatureNormal;

    HalfEdge* he;

    Eigen::Vector3d pos;
    Eigen::Vector3d normal;

    std::map<float, Eigen::Vector3d> timeframePos;
    char desc[64];

    Eigen::Vector3d originalPos; // Original constraint
    // Eigen::Matrix4d deformedRot;
public:
    Vertex() :
        he(nullptr) {}
    ~Vertex() {}

    Eigen::Vector3d getInterpolatedPos(float time);
};

// Edge Data ======================================================================================
class Edge {
public:
    unsigned int index;
    double weight;

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
    double angle;

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
};

#endif // MESH_DATA_HPP
