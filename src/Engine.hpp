#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer.hpp"
#include "Trackball.hpp"

class Engine
{
private:
    static Engine* instance;

    int m_screenWidth = 800;
    int m_screenHeight = 600;
    GLFWwindow* m_window;

    Renderer* m_renderer;
    Trackball* m_trackball;

public:
    Engine();
    ~Engine();

    void update();
    void run();

public:
    static void resizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif // ENGINE_HPP
