#include "Plane.hpp"

Object::Plane::Plane(Shader* shader) : Base(shader)
{
    init();
}

Object::Plane::~Plane()
{

}

void Object::Plane::init()
{
    init(10, 10);
}

void Object::Plane::init(int size, int segments)
{
    this->size = size;
    this->segments = segments;

    float halfSize = size * 0.5f;
        float step = size / float(segments);

        vertices.clear();
        indices.clear();

        int index = 0;
        // Z Axis
        for (int i = 0; i <= segments; ++i) {
            float x = -halfSize + i * step;

            vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(-halfSize);
            vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(halfSize);

            indices.push_back(index++);
            indices.push_back(index++);
        }
        // X Axis
        for (int i = 0; i <= segments; ++i) {
            float z = -halfSize + i * step;

            vertices.push_back(-halfSize); vertices.push_back(0.0f); vertices.push_back(z);
            vertices.push_back(halfSize);  vertices.push_back(0.0f); vertices.push_back(z);

            indices.push_back(index++);
            indices.push_back(index++);
        }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

}

void Object::Plane::draw(Eigen::Matrix4f projection, Eigen::Matrix4f view)
{
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
}
