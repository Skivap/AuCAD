#ifndef AXIS_HPP
#define AXIS_HPP

#include "BaseObject.hpp"
#include "Mesh.hpp"

namespace Object
{
    class Axis : public Base
    {
    private:
        Mesh* pc;
        Eigen::Vector3f m_translation;
    public:
        Axis(Shader* shader, float radius, float radius_scale, float scale, float offset, int segments);
        ~Axis();

        void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) override;
        void draw(const CameraParam& cameraParam) override;

        void setTranslation(const Eigen::Vector3f& t) { m_translation = t; }
    };
}

#endif // AXIS_HPP
