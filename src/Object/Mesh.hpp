#ifndef OBJECT_MESH_HPP
#define OBJECT_MESH_HPP

#include "Utilities/HalfEdge.hpp"
#include "BaseObject.hpp"
#include "Wireframe.hpp"

namespace Object {
    class Mesh : public Base {
    private:
        // Data Structure for Half Edges
        std::vector<HalfEdge> m_halfEdges;
        std::vector<Triangle> m_triangles;
        // std::vector<Edge> m_edges;
        std::vector<Vertex> m_vertices;

    public:
        Mesh(Shader* shader,
            const std::vector<Eigen::Vector3f>& vertices,
            const std::vector<Eigen::Vector3f>& normals,
            const std::vector<Eigen::Vector3i>& indices);
        ~Mesh();

        void init() override;
        void draw(const CameraParam& cameraParam) override;

        Wireframe* createWireframe(Shader* wireframeShader);
        static std::vector<Mesh*> loadMeshes(Shader* shader, const std::string& filePath);
    };
}

#endif // OBJECT_MESH_HPP
