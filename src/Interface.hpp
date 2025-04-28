#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Interface {
private:
    GLFWwindow* m_window;
    int m_width, m_height;
public:
    Interface(GLFWwindow* window, int screen_width, int screen_height);
    ~Interface();

    void resize(int width, int height);
    void draw();
};

#endif // INTERFACE_HPP
