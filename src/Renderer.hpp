#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <Eigen/Dense>

#include "Object/BaseObject.hpp"
#include "Object/Wireframe.hpp"
#include "Object/Mesh.hpp"

#include "Utilities/Shader.hpp"

class Renderer {
private:
    Object::Wireframe* m_plane;

    Object::Mesh* m_mesh;

    Shader* m_wireframe;
    Shader* m_meshShader;

    int m_screenHeight, m_screenWidth;
public:
    Renderer();
    ~Renderer();

    void initShaders();
    void initModels();

    void draw(const CameraParam& cameraParam);

public:
};

#endif // RENDERER_HPP
