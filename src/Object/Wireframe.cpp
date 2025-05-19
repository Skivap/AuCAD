#include "Wireframe.hpp"

Object::Wireframe::Wireframe(Shader* shader, int size, int segments) : Base(shader)
{
    m_baseColor = Eigen::Vector3f(1.0f, 1.0f, 1.0f);

    float halfSize = size * 0.5f;
    float step = size / float(segments);

    std::vector<float> buffer;
    std::vector<unsigned int> indices;

    // Z Axis lines
    for (int i = 0; i <= segments; ++i) {
        float x = -halfSize + i * step;

        buffer.push_back(x);
        buffer.push_back(0.0f);
        buffer.push_back(-halfSize);
        buffer.push_back(m_baseColor.x());
        buffer.push_back(m_baseColor.y());
        buffer.push_back(m_baseColor.z());

        buffer.push_back(x);
        buffer.push_back(0.0f);
        buffer.push_back(halfSize);
        buffer.push_back(m_baseColor.x());
        buffer.push_back(m_baseColor.y());
        buffer.push_back(m_baseColor.z());
    }

    // X Axis lines
    for (int i = 0; i <= segments; ++i) {
        float z = -halfSize + i * step;

        buffer.push_back(-halfSize);
        buffer.push_back(0.0f);
        buffer.push_back(z);
        buffer.push_back(m_baseColor.x());
        buffer.push_back(m_baseColor.y());
        buffer.push_back(m_baseColor.z());

        buffer.push_back(halfSize);
        buffer.push_back(0.0f);
        buffer.push_back(z);
        buffer.push_back(m_baseColor.x());
        buffer.push_back(m_baseColor.y());
        buffer.push_back(m_baseColor.z());
    }

    init(buffer, indices);

    bufferSize = buffer.size();
    indicesSize = indices.size();
}

Object::Wireframe::Wireframe(Shader* shader, const std::vector<Eigen::Vector3f>& _vertices,
                             const std::vector<Eigen::Vector2i>& _indices) : Base(shader)
{
    m_baseColor = Eigen::Vector3f(1.0f, 1.0f, 1.0f);

    std::vector<float> buffer;
    std::vector<unsigned int> indices;

    for (const auto& idx : _indices) {
        const auto& v1 = _vertices[idx.x()];
        const auto& v2 = _vertices[idx.y()];
        buffer.push_back(v1.x());
        buffer.push_back(v1.y());
        buffer.push_back(v1.z());
        buffer.push_back(m_baseColor.x());
        buffer.push_back(m_baseColor.y());
        buffer.push_back(m_baseColor.z());

        buffer.push_back(v2.x());
        buffer.push_back(v2.y());
        buffer.push_back(v2.z());
        buffer.push_back(m_baseColor.x());
        buffer.push_back(m_baseColor.y());
        buffer.push_back(m_baseColor.z());
    }

    init(buffer, indices);

    bufferSize = buffer.size();
    indicesSize = indices.size();
}

Object::Wireframe::~Wireframe()
{
    // cleanup if needed
}

void Object::Wireframe::init(std::vector<float>& buffer, std::vector<unsigned int>& indices)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Object::Wireframe::draw(const CameraParam& cameraParam)
{
    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    shader->setMat4("model", modelMatrix);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, bufferSize / 6);
}
