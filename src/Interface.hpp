#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

class MeshData;
namespace GenAPI {
    class DeformationGenerator;
    struct GenerationResponse;
}

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
    
    // Generation panel state
    std::unique_ptr<GenAPI::DeformationGenerator> m_generator;
    bool m_showGenerationPanel;
    char m_promptBuffer[256];
    int m_animationLength;
    std::string m_apiUrl;
    bool m_isGenerating;
    std::string m_lastError;
    bool m_apiConnected;
    
    // ARAP all frames processing state
    bool m_processingAllFrames;
    int m_currentProcessingFrame;
    int m_totalFramesToProcess;
public:
    Interface(GLFWwindow* window, int screen_width, int screen_height);
    ~Interface();

    void resize(int width, int height);
    void draw();
    void setMeshData(MeshData* meshData) { m_meshData = meshData; }
    
private:
    void drawVertexPanel();
    void drawGenerationPanel();
    void checkApiConnection();
    void generateDeformations();

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
