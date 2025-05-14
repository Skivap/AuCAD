#include "Wireframe.hpp"

Object::Wireframe::Wireframe(Shader* shader, int size, int segments) : Base(shader)
{
    float halfSize = size * 0.5f;
    float step = size / float(segments);

    m_buffer.clear();
    m_indices.clear();

    int index = 0;
    // Z Axis
    for (int i = 0; i <= segments; ++i) {
        float x = -halfSize + i * step;

        m_buffer.push_back(x); m_buffer.push_back(0.0f); m_buffer.push_back(-halfSize);
        m_buffer.push_back(x); m_buffer.push_back(0.0f); m_buffer.push_back(halfSize);

        m_indices.push_back(index++);
        m_indices.push_back(index++);
    }
    // X Axis
    for (int i = 0; i <= segments; ++i) {
        float z = -halfSize + i * step;

        m_buffer.push_back(-halfSize); m_buffer.push_back(0.0f); m_buffer.push_back(z);
        m_buffer.push_back(halfSize);  m_buffer.push_back(0.0f); m_buffer.push_back(z);

        m_indices.push_back(index++);
        m_indices.push_back(index++);
    }

    init();
}

Object::Wireframe::Wireframe(Shader* shader, const std::vector<Eigen::Vector3f>& vertices,
                             const std::vector<Eigen::Vector2i>& indices) : Base(shader)
{
    for(int i = 0; i < vertices.size(); i++) {
        m_buffer.push_back(vertices[i].x());
        m_buffer.push_back(vertices[i].y());
        m_buffer.push_back(vertices[i].z());
    }

    for(int i = 0; i < indices.size(); i++) {
        m_indices.push_back(indices[i].x());
        m_indices.push_back(indices[i].y());
    }

    init();
}

Object::Wireframe::~Wireframe()
{

}

void Object::Wireframe::init()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_buffer.size() * sizeof(float), m_buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Object::Wireframe::draw(const CameraParam& cameraParam)
{
    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_INT, 0);
}
