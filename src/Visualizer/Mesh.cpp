#include "Mesh.hpp"


Object::Mesh::Mesh(Shader* shader,
                   const std::vector<Eigen::Vector3f>& vertices,
                   const std::vector<Eigen::Vector3f>& normals,
                   const std::vector<Eigen::Vector3i>& indices)
    : Base(shader) {

    if (vertices.size() != normals.size()) {
        std::cout << "Vertices and Normals doesn't match\n";
    }

    std::vector<float> buffer;
    std::vector<float> bufferPositions, bufferNormals, bufferColors;

    for (const auto& tri : indices) {
        for (int j = 0; j < 3; ++j) {
            int idx = tri[j];
            const auto& v = vertices[idx];
            const auto& n = normals[idx];

            bufferPositions.push_back(v.x());
            bufferPositions.push_back(v.y());
            bufferPositions.push_back(v.z());

            bufferNormals.push_back(n.x());
            bufferNormals.push_back(n.y());
            bufferNormals.push_back(n.z());

            bufferColors.push_back(0.0f);
            bufferColors.push_back(0.0f);
            bufferColors.push_back(0.0f);
        }
    }

    buffer.clear();
    buffer.insert(buffer.end(), bufferPositions.begin(), bufferPositions.end());
    buffer.insert(buffer.end(), bufferNormals.begin(), bufferNormals.end());
    buffer.insert(buffer.end(), bufferColors.begin(), bufferColors.end());

    // INDICES ONLY FOR WIREFRAME SETUP!!!
    std::vector<unsigned int> bufferIndices;
    for(int i = 0; i < indices.size(); i++) {
        bufferIndices.push_back(indices[i].x());
        bufferIndices.push_back(indices[i].y());
        bufferIndices.push_back(indices[i].z());
    }

    bufferSize = buffer.size();
    indicesSize = 0;

    init(buffer, bufferIndices);
}

Object::Mesh::~Mesh() {}

void Object::Mesh::init(std::vector<float>& buffer, std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

    size_t vertexCount = buffer.size() / 9;
    size_t verticesOffset = 0;
    size_t normalsOffset = vertexCount * 3 * sizeof(float);
    size_t colorsOffset  = vertexCount * 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)verticesOffset);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalsOffset);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)colorsOffset);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


void Object::Mesh::draw(const CameraParam& cameraParam) {
    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    shader->setVec3("camPos", cameraParam.position);
    shader->setMat4("model", modelMatrix);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, bufferSize / 9);
}
