#include "Engine.hpp"

#include <iostream>

Engine* Engine::instance = nullptr;

Engine::Engine() : m_renderer(), m_trackball()
{
    instance = this;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, "LearnOpenGL", NULL, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::terminate(); // TODO: Change handle error
    }

    // Callbacks
    glfwMakeContextCurrent(m_window);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetFramebufferSizeCallback(m_window, resizeCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::terminate(); // TODO: Change handle error
    }

    m_renderer = new Renderer();
    m_trackball = new Trackball();
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
}

void Engine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        instance->m_trackball->startDrag(xpos, ypos);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        instance->m_trackball->endDrag(xpos, ypos);
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

void Engine::run()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(m_window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
        const Eigen::Matrix4f& proj = m_trackball->getProjectionMatrix();
        const Eigen::Matrix4f& view = m_trackball->getViewMatrix();
        m_renderer->draw(proj, view);


        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }

    glfwTerminate();
    return;
}
