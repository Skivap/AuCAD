#include "Wireframe.hpp"

Object::Wireframe::Wireframe(Shader* shader, int size, int segments) : Base(shader)
{
    float halfSize = size * 0.5f;
    float step = size / float(segments);

    std::vector<float> bufferPos;
    std::vector<float> bufferColor;

    std::vector<unsigned int> indices;

    // Z Axis lines
    for (int i = 0; i <= segments; ++i) {
        float x = -halfSize + i * step;

        bufferPos.push_back(x);
        bufferPos.push_back(0.0f);
        bufferPos.push_back(-halfSize);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);

        bufferPos.push_back(x);
        bufferPos.push_back(0.0f);
        bufferPos.push_back(halfSize);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);
    }

    // X Axis lines
    for (int i = 0; i <= segments; ++i) {
        float z = -halfSize + i * step;

        bufferPos.push_back(-halfSize);
        bufferPos.push_back(0.0f);
        bufferPos.push_back(z);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);

        bufferPos.push_back(halfSize);
        bufferPos.push_back(0.0f);
        bufferPos.push_back(z);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);
        bufferColor.push_back(1.0f);
    }

    std::vector<float> buffer;
    buffer.insert(buffer.end(), bufferPos.begin(), bufferPos.end());
    buffer.insert(buffer.end(), bufferColor.begin(), bufferColor.end());

    init(buffer, indices);

    bufferSize = buffer.size();
    indicesSize = indices.size();
}

Object::Wireframe::Wireframe(Shader* shader, const std::vector<Eigen::Vector3f>& _vertices,
                             const std::vector<Eigen::Vector2i>& _indices) : Base(shader)
{

    std::vector<float> bufferPos;
    std::vector<float> bufferColor;
    std::vector<unsigned int> indices;

    for (const auto& idx : _indices) {
        const auto& v1 = _vertices[idx.x()];
        const auto& v2 = _vertices[idx.y()];
        bufferPos.push_back(v1.x());
        bufferPos.push_back(v1.y());
        bufferPos.push_back(v1.z());
        bufferColor.push_back(0.0f);
        bufferColor.push_back(0.0f);
        bufferColor.push_back(0.0f);

        bufferPos.push_back(v2.x());
        bufferPos.push_back(v2.y());
        bufferPos.push_back(v2.z());
        bufferColor.push_back(0.0f);
        bufferColor.push_back(0.0f);
        bufferColor.push_back(0.0f);
    }

    std::vector<float> buffer;
    buffer.insert(buffer.end(), bufferPos.begin(), bufferPos.end());
    buffer.insert(buffer.end(), bufferColor.begin(), bufferColor.end());

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(buffer.size() * sizeof(float) / 2));
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
