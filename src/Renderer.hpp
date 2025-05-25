#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <Eigen/Dense>

#include "Visualizer/BaseObject.hpp"

#include "Mesh/MeshData.hpp"
#include "Gizmo/Gizmo.hpp"

#include "Utilities/Shader.hpp"

class Renderer {
private:
    MeshData* m_meshData;
    Gizmo* m_gizmo;

    Object::Wireframe* m_plane;

    Shader* m_wireframeShader;
    Shader* m_meshShader;
    Shader* m_axisShader;

    int m_screenHeight, m_screenWidth;
public:
    Renderer();
    ~Renderer();

    void initShaders();
    void initModels();

    void draw(const CameraParam& cameraParam);

    MeshData* getMeshData() { return m_meshData; }
    Gizmo* getGizmo() { return m_gizmo; }

public:
};

#endif // RENDERER_HPP
