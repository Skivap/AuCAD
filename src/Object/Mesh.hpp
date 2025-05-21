#ifndef OBJECT_MESH_HPP
#define OBJECT_MESH_HPP

#include "Utilities/MeshData.hpp"
#include "BaseObject.hpp"
#include "Wireframe.hpp"
#include "PointCloud.hpp"

namespace Object {
    class Mesh : public Base {
    private:
        MeshData* meshData;

        PointCloud* m_pointCloud;
        Wireframe* m_wireframe;

        bool m_drawWireframe;
        bool m_drawPointCloud;

    public:
        Mesh(Shader* shader, Shader* wireframe_shader,
            const std::vector<Eigen::Vector3f>& vertices,
            const std::vector<Eigen::Vector3f>& normals,
            const std::vector<Eigen::Vector3i>& indices);
        ~Mesh();

        void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) override;
        void draw(const CameraParam& cameraParam) override;

        void initWireframe(Shader* shader);
        void initPointCloud(Shader* shader);
        static std::vector<Mesh*> loadMeshes(Shader* shader, Shader* wireframe_shader, const std::string& filePath);

        MeshData* getMeshData() { return meshData; };
    };
}

#endif // OBJECT_MESH_HPP
