#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <Eigen/Dense>
#include <vector>

#include "Object/BaseObject.hpp"
#include "Utilities/Shader.hpp"

class Renderer {
private:
    std::vector<Object::Base*> m_models;
    std::vector<Shader*> m_shaders;
public:
    Renderer();
    ~Renderer();

    void initShaders();
    void initModels();

    void draw(const Eigen::Matrix4f& projection, const Eigen::Matrix4f& view);
};

#endif // RENDERER_HPP
