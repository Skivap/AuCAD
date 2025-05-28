#include "Renderer.hpp"

Renderer::Renderer()
{
    std::cout << "Initializing Shaders" << std::endl;
    initShaders();
    std::cout << "Initializing Models" << std::endl;
    initModels();
    std::cout << "Finish Initialization" << std::endl;


    std::cout << "Eigen version: "
              << EIGEN_WORLD_VERSION << "."
              << EIGEN_MAJOR_VERSION << "."
              << EIGEN_MINOR_VERSION << std::endl;
    // return 0;
}

Renderer::~Renderer()
{
    delete m_meshData;
    delete m_plane;
    delete m_gizmo;
}

void Renderer::initShaders()
{
    m_wireframeShader = new Shader("./shaders/default.vs.glsl", "./shaders/default.fs.glsl");
    m_meshShader = new Shader("./shaders/shader.vs.glsl", "./shaders/shader.fs.glsl");
    m_axisShader = new Shader("./shaders/axis.vs.glsl", "./shaders/axis.fs.glsl");
}

void Renderer::initModels()
{
    m_meshData = new MeshData(m_meshShader, m_wireframeShader, m_meshShader, "./assets/bunny.ply");
    m_plane = new Object::Wireframe(m_wireframeShader);
    m_gizmo = new Gizmo(m_axisShader);
}

void Renderer::draw(const CameraParam& cameraParam)
{
    // m_plane->draw(cameraParam);
    m_meshData->draw(cameraParam);
    if (m_meshData->getLastSelectedVertex() != -1) {
        m_gizmo->draw(cameraParam);
    }
}
