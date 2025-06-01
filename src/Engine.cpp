#include "Engine.hpp"

#include <iostream>

Engine* Engine::instance = nullptr;

Engine::Engine() : m_renderer(), m_trackball(), m_isDraggingAxis(false)
{
    instance = this;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, "Geometric Modeling", NULL, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::terminate(); // TODO: Change handle error
    }

    // Callbacks
    glfwMakeContextCurrent(m_window);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetWindowSizeCallback(m_window, resizeCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::terminate(); // TODO: Change handle error
    }

    m_renderer = new Renderer();
    m_trackball = new Trackball(m_screenWidth, m_screenHeight);
    m_interface = new Interface(m_window, m_screenWidth, m_screenHeight);
    
    // Set mesh data reference in interface
    m_interface->setMeshData(m_renderer->getMeshData());
}

Engine::~Engine()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::resizeCallback(GLFWwindow* window, int width, int height)
{
    instance->m_screenWidth = width;
    instance->m_screenHeight = height;
    glViewport(0, 0, width, height);

    instance->m_interface->resize(width, height);
    instance->m_trackball->resize(width, height);
}

void Engine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if(!instance->m_interface->isHovered())
            instance->m_trackball->startDrag(xpos, ypos);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        if(!instance->m_interface->isHovered())
            instance->m_trackball->endDrag(xpos, ypos);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // float depth;
        // glReadPixels(xpos, instance->m_screenHeight - ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        Eigen::Vector3f screenCoord(static_cast<float>(xpos), static_cast<float>(ypos), 0);
        Eigen::Vector3f nearCoord = instance->m_trackball->unProject(screenCoord);

        Interface::SelectionMode mode = instance->m_interface->getSelectionMode();
        MeshData* meshData = instance->m_renderer->getMeshData();
        Gizmo* gizmo = instance->m_renderer->getGizmo();

        switch(mode) {
            case Interface::SelectionMode::Triangle:
                {
                    meshData->selectTriangle(instance->m_trackball->getPosition(), nearCoord);
                    break;
                }
            case Interface::SelectionMode::Vertex:
                {
                    int idx = meshData->selectVertex(instance->m_trackball->getPosition(), nearCoord);
                    if (idx != -1) {
                        gizmo->setTranslation(meshData->getVertex(idx).pos.cast<float>());
                    }
                    break;
                }
            case Interface::SelectionMode::AxisDebug:
                {
                    const int idx = meshData->getLastSelectedVertex();
                    if(idx != -1) {
                        gizmo->pickTranslation(instance->m_trackball->getPosition(), nearCoord);
                        instance->m_isDraggingAxis = true;
                    }
                    break;
                }
            default:
                break;
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        Gizmo* gizmo = instance->m_renderer->getGizmo();
        gizmo->clearSelection();
        instance->m_isDraggingAxis = false;
    }
}

void Engine::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    instance->m_trackball->drag(xpos, ypos);
}

void Engine::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    instance->m_trackball->zoom(yoffset);
}

void Engine::update() {
    static int lastMode = 1;

    lastMode = m_interface->getVisualizeMode() != 0 ? m_interface->getVisualizeMode() : lastMode;

    MeshData* meshData = m_renderer->getMeshData();
    if(m_interface->getVisualizeMode() == 1) {
        meshData->refreshTriangleColor(MeshVisMode::None);
    }
    else if(m_interface->getVisualizeMode() == 2) {
        meshData->refreshTriangleColor(MeshVisMode::Normals);
    }
    else if(m_interface->getVisualizeMode() == 3) {
        meshData->refreshTriangleColor(MeshVisMode::Weight);
    }

    if (m_interface->getCompute()) {
        meshData->computeARAP();
        // meshData->computeLaplacianSurfaceModeling();
    }

    if(instance->m_isDraggingAxis == true) {
        // Axis Logic Update
        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        Eigen::Vector3f screenCoord(static_cast<float>(xpos), static_cast<float>(ypos), 0);
        Eigen::Vector3f nearCoord = instance->m_trackball->unProject(screenCoord);
        Gizmo* gizmo = instance->m_renderer->getGizmo();
        gizmo->dragAlongAxis(m_trackball->getPosition(), nearCoord);

        const Eigen::Vector3f& t = gizmo->getTranslation();
        int idx = meshData->getLastSelectedVertex();
        if(idx != -1) {
            meshData->changeVertexPosition(idx, t);
        }
    }

    if(instance->m_interface->getDoRefresh()) {
        Gizmo* gizmo = instance->m_renderer->getGizmo();
        gizmo->clearSelection();
        meshData->refreshPosition(m_interface->getTimeFrame());
    }

    if(m_interface->getSetTimeFrame()) {
        meshData->saveTimeFrame(m_interface->getTimeFrame());
    }

    const int idx = meshData->getLastSelectedVertex();
    if(idx != -1) {
        Vertex& v = meshData->getVertex(idx);
        m_interface->setBuffer(v.desc);
    }
    else {
        char buf[64] = "Select null vertex";
        m_interface->setBuffer(buf);
    }
}

void Engine::run()
{
    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    while (!glfwWindowShouldClose(m_window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update
        update();

        // Draw
        const CameraParam cameraParam(
            m_trackball->getProjectionMatrix(),
            m_trackball->getViewMatrix(),
            m_trackball->getPosition()
        );
        m_renderer->draw(cameraParam);
        m_interface->draw();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    glfwTerminate();
    return;
}
