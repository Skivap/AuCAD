#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class MeshData;

class Interface {
private:
    GLFWwindow* m_window;
    int m_width, m_height;

    int m_visualizeMode; // TODO: Find a better way for the UI to visualize
    bool m_computeDeformedPos;

    float m_weightThreshold;
    float timestep;
    bool safeTimeframe;
    bool doRefresh;
    char * buffer;
    
    // Vertex panel state
    MeshData* m_meshData;
    bool m_showVertexPanel;
public:
    Interface(GLFWwindow* window, int screen_width, int screen_height);
    ~Interface();

    void resize(int width, int height);
    void draw();
    void setMeshData(MeshData* meshData) { m_meshData = meshData; }
    
private:
    void drawVertexPanel();

public:
    enum SelectionMode{
        None        = 0x0,
        Triangle    = 0x1,
        Vertex      = 0x2,
        AxisDebug   = 0x3
    };

private:
    bool m_wireframe;
    SelectionMode m_selectionMode;

public:
    const SelectionMode getSelectionMode() { return m_selectionMode; }
    const int getVisualizeMode(){ return m_visualizeMode; }
    const bool getCompute() { return m_computeDeformedPos; }

    const bool getDoRefresh() { return doRefresh; }
    const bool getSetTimeFrame() { return safeTimeframe; }
    const float getWeight() { return m_weightThreshold; }
    const float getTimeFrame() { return timestep; }

    const bool isHovered();

    void setBuffer(char * b) { buffer = b; }
};

#endif // INTERFACE_HPP
