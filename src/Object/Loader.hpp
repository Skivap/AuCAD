#ifndef OBJECT_LOADER_HPP
#define OBJECT_LOADER_HPP

#include "BaseObject.hpp"

namespace Object
{
    class Loader : public Base
    {
    private:
        Shader* shader;
    public:
        Loader(Shader* shader, std::string filename);
        ~Loader();

        void init() override;
        void draw(Eigen::Matrix4f projection, Eigen::Matrix4f view) override;
    };
}

#endif // OBJECT_LOADER_HPP
