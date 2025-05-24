#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <Eigen/Dense>

#include "Object/BaseObject.hpp"
#include "Object/Wireframe.hpp"
#include "Object/Mesh.hpp"
#include "Object/Axis.hpp"

#include "Utilities/Shader.hpp"

class Renderer {
private:
    Object::Wireframe* m_plane;
    Object::Mesh* m_mesh;
    Object::Axis* m_axis;

    Shader* m_wireframe;
    Shader* m_meshShader;
    Shader* m_axisShader;

    int m_screenHeight, m_screenWidth;
public:
    Renderer();
    ~Renderer();

    void initShaders();
    void initModels();

    void draw(const CameraParam& cameraParam);

    MeshData* getMeshData() { return m_mesh->getMeshData(); }
    Gizmo* getGizmo() { return m_axis->getGizmo(); }

public:
};

#endif // RENDERER_HPP
