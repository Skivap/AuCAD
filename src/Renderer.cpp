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
    m_planeShader = new Shader("./shaders/plane.vs.glsl", "./shaders/plane.fs.glsl");
    m_meshShader = new Shader("./shaders/shader.vs.glsl", "./shaders/shader.fs.glsl");
}

void Renderer::initModels()
{
    m_plane = new Object::Wireframe(m_planeShader);
    m_mesh = Object::Mesh::loadMeshes(m_meshShader, "./assets/bunny.ply")[0];
}

void Renderer::draw(const CameraParam& cameraParam)
{
    m_plane->draw(cameraParam);
    m_mesh->draw(cameraParam);
}
