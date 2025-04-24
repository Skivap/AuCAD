#include "Renderer.hpp"
#include "Object/Plane.hpp"

Renderer::Renderer()
{
    std::cout << "Initializing Shaders" << std::endl;
    initShaders();
    std::cout << "Initializing Models" << std::endl;
    initModels();
    std::cout << "Finish Initialization" << std::endl;
}

Renderer::~Renderer()
{
    for (auto model : m_models) {
        delete model;
    }
}

void Renderer::initShaders()
{
    m_shaders.push_back(new Shader("./shaders/shader.vs.glsl", "./shaders/shader.fs.glsl"));
}

void Renderer::initModels()
{
    m_models.push_back(new Object::Plane(m_shaders[0]));
}

void Renderer::draw(const Eigen::Matrix4f& projection, const Eigen::Matrix4f& view)
{
    for (auto model : m_models)
    {
        model->draw(projection, view);
    }
}
