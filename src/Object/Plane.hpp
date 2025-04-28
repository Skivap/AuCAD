#ifndef PLANE_HPP
#define PLANE_HPP

#include "BaseObject.hpp"

namespace Object
{
    class Plane : public Base
    {
    private:
        int size, segments;

    public:
        Plane(Shader* shader);
        ~Plane();

        void init() override;
        void init(int size, int segments);
        void draw(const CameraParam& cameraParam) override;
    };
}

#endif // PLANE_HPP
