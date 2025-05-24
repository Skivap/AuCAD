#include "Renderer.hpp"

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
    delete m_mesh;
    delete m_plane;
}

void Renderer::initShaders()
{
    m_wireframe = new Shader("./shaders/default.vs.glsl", "./shaders/default.fs.glsl");
    m_meshShader = new Shader("./shaders/shader.vs.glsl", "./shaders/shader.fs.glsl");
    m_axisShader = new Shader("./shaders/axis.vs.glsl", "./shaders/axis.fs.glsl");
}

void Renderer::initModels()
{
    m_plane = new Object::Wireframe(m_wireframe);
    m_mesh = Object::Mesh::loadMeshes(m_meshShader, m_wireframe, "./assets/bunny.ply")[0];
    m_axis = new Object::Axis(m_meshShader);
}

void Renderer::draw(const CameraParam& cameraParam)
{
    m_plane->draw(cameraParam);
    m_mesh->draw(cameraParam);
    m_axis->draw(cameraParam);
}
