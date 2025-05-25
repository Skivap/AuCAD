#ifndef OBJECT_MESH_HPP
#define OBJECT_MESH_HPP

#include "BaseObject.hpp"

namespace Object {
    class Mesh : public Base {
    private:

    public:
        Mesh(Shader* shader,
            const std::vector<Eigen::Vector3f>& vertices,
            const std::vector<Eigen::Vector3f>& normals,
            const std::vector<Eigen::Vector3i>& indices);
        ~Mesh();

        void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) override;
        void draw(const CameraParam& cameraParam) override;

        static std::vector<Mesh*> loadMeshes(Shader* shader, Shader* wireframe_shader, const std::string& filePath);
    };
}

#endif // OBJECT_MESH_HPP
