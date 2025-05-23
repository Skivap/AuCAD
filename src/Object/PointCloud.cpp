#include "PointCloud.hpp"


Object::PointCloud::PointCloud(Shader* shader, MeshData* meshData, const std::vector<Eigen::Vector3f>& vertices)
: Base(shader), m_selectedColor(1.0f, 0.0f, 0.0f), m_baseColor(0.0f, 0.0f, 0.0f), m_meshData(meshData)
{
    m_offsets = vertices;

    const int rings = 16;
    const int segments = 16;
    const float radius = 0.005f;

    std::vector<float> buffer;
    std::vector<unsigned int> indices;

    for (int ring = 0; ring <= rings; ring++) {
        float phi = M_PI * float(ring) / float(rings);
        for (int segment = 0; segment <= segments; segment++) {
            float theta = 2.0f * M_PI * float(segment) / float(segments);

            // Calculate vertex position
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            // // Calculate normal
            float nx = sin(phi) * cos(theta);
            float ny = cos(phi);
            float nz = sin(phi) * sin(theta);

            // Position
            buffer.push_back(x);
            buffer.push_back(y);
            buffer.push_back(z);
            // Normal
            buffer.push_back(nx);
            buffer.push_back(ny);
            buffer.push_back(nz);
            // Color
            buffer.push_back(0.0f);
            buffer.push_back(0.0f);
            buffer.push_back(0.0f);
        }
    }

    // Generate indices
    for (int ring = 0; ring < rings; ring++) {
        for (int segment = 0; segment < segments; segment++) {
            unsigned int current = ring * (segments + 1) + segment;
            unsigned int next = current + segments + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(next);
            indices.push_back(next + 1);
            indices.push_back(current + 1);
        }
    }

    init(buffer, indices);

    bufferSize = buffer.size();
    indicesSize = indices.size();
}

Object::PointCloud::~PointCloud()
{

}

void Object::PointCloud::init(std::vector<float>& buffer, std::vector<unsigned int>& indices)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Object::PointCloud::draw(const CameraParam& cameraParam)
{
    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    glBindVertexArray(VAO);

    const std::vector<bool> selected = m_meshData->getSelectedVertices();


    for(int i = 0; i < m_offsets.size(); i++) {
        if(!selected[i]) continue;
        Eigen::Vector3f& offset = m_offsets[i];

        reset();
        translate(offset);
        shader->setMat4("model", modelMatrix);
        glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
    }
}
