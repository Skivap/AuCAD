#ifndef AXIS_HPP
#define AXIS_HPP

#include "BaseObject.hpp"
#include "Gizmo/Gizmo.hpp"

namespace Object
{
    class Axis : public Base
    {
    private:
        Gizmo* m_gizmo;

    public:
        Axis(Shader* shader);
        ~Axis();

        void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) override;
        void draw(const CameraParam& cameraParam) override;

        Gizmo* getGizmo() { return m_gizmo; }
    };
}

#endif // AXIS_HPP
