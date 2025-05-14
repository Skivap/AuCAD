#ifndef WIREFRAME_HPP
#define WIREFRAME_HPP

#include "BaseObject.hpp"

namespace Object
{
    class Wireframe : public Base
    {
    private:

    public:
        Wireframe(Shader* shader, int size = 10, int segments = 10); // Plane Option
        Wireframe(Shader* shader, const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector2i>& indices); // Given Vertices and Indices Option
        ~Wireframe();

        void init() override;
        void draw(const CameraParam& cameraParam) override;
    };
}

#endif // WIREFRAME_HPP
