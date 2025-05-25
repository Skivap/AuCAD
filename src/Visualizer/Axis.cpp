#include "Axis.hpp"

Object::Axis::Axis(Shader* shader, float radius, float radius_scale, float scale, float offset, int segments) : Base(shader) {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> colors;
    std::vector<unsigned int> indices;

    Eigen::Vector3f color[3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    Eigen::Vector3f axisDirection[3] = {
        {scale, 0.0f, 0.0f},
        {0.0f, scale, 0.0f},
        {0.0f, 0.0f, scale}
    };

    float rad[] = {radius, radius, radius * radius_scale};
    float t[] = {0.0f, offset, offset};

    for (int ax = 0; ax < 3; ax++) {
        int baseIdx = ax * (3 * segments + 2);

        for (int i = 0; i < 3; i++) {
            Eigen::Vector3f offset = axisDirection[ax] * t[i];

            for (int j = 0; j < segments; j++) {
                float theta = 2.0f * M_PI * float(j) / float(segments);
                Eigen::Vector3f pt = cos(theta) * axisDirection[(ax + 1) % 3] * rad[i] + sin(theta) * axisDirection[(ax + 2) % 3] * rad[i];
                vertices.push_back(offset.x() + pt.x()); vertices.push_back(offset.y() + pt.y()); vertices.push_back(offset.z() + pt.z());
                pt = pt.normalized();
                normals.push_back(pt[0]); normals.push_back(pt[1]); normals.push_back(pt[2]);
                colors.push_back(color[ax].x()); colors.push_back(color[ax].y()); colors.push_back(color[ax].z());
            }
        }

        // Indices
        for (int i = 1; i < 3; i++) {
            for (int j = 0; j < segments; j++) {
                int p0 = (i - 1) * segments + (j - 1 + segments) % segments;
                int p1 = (i - 1) * segments + j;
                int p2 = i * segments + (j - 1 + segments) % segments;
                int p3 = i * segments + j;

                indices.push_back(baseIdx + p1); indices.push_back(baseIdx + p0); indices.push_back(baseIdx + p2);
                indices.push_back(baseIdx + p1); indices.push_back(baseIdx + p2); indices.push_back(baseIdx + p3);
            }
        }

        // Create circle
        Eigen::Vector3f normal_vector = axisDirection[ax].normalized();
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
        normals.push_back(-normal_vector.x()); normals.push_back(-normal_vector.y()); normals.push_back(-normal_vector.z());
        colors.push_back(color[ax].x()); colors.push_back(color[ax].y()); colors.push_back(color[ax].z());

        vertices.push_back(axisDirection[ax].x()); vertices.push_back(axisDirection[ax].y()); vertices.push_back(axisDirection[ax].z());
        normals.push_back(normal_vector.x()); normals.push_back(normal_vector.y()); normals.push_back(normal_vector.z());
        colors.push_back(color[ax].x()); colors.push_back(color[ax].y()); colors.push_back(color[ax].z());

        // Set Indices
        for (int i = 0; i < segments; i++) {
            indices.push_back(baseIdx + i);
            indices.push_back(baseIdx + (i + 1) % segments);
            indices.push_back(baseIdx + segments * 3);

            indices.push_back(baseIdx + segments * 2 + i);
            indices.push_back(baseIdx + segments * 2 + ((i + 1) % segments));
            indices.push_back(baseIdx + segments * 3 + 1);
        }
    }

    std::vector<float> buffer;
    buffer.insert(buffer.end(), vertices.begin(), vertices.end());
    buffer.insert(buffer.end(), normals.begin(), normals.end());
    buffer.insert(buffer.end(), colors.begin(), colors.end());

    init(buffer, indices);

    bufferSize = buffer.size();
    indicesSize = indices.size();
}

Object::Axis::~Axis() {

}

void Object::Axis::init(std::vector<float>& buffer, std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(buffer.size() * sizeof(float) / 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(buffer.size() * sizeof(float) * 2 / 3));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Object::Axis::draw(const CameraParam& cameraParam) {
    reset();
    translate(m_translation);

    shader->use();
    shader->setMat4("projection", cameraParam.projection);
    shader->setMat4("view", cameraParam.view);
    shader->setMat4("model", modelMatrix);

    glDepthRange(0.0, 0.01);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indicesSize), GL_UNSIGNED_INT, 0);
    glDepthRange(0.0, 1.0);

    // Helper to turn an error code into a human-readable string
    auto GetGLErrorString = [](GLenum err) {
        switch (err) {
            case GL_NO_ERROR:                      return "GL_NO_ERROR";
            case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
            case GL_STACK_OVERFLOW:                return "GL_STACK_OVERFLOW";
            case GL_STACK_UNDERFLOW:               return "GL_STACK_UNDERFLOW";
            case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
            case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
            default:                               return "Unknown GL error";
        }
    };

    // Call this right after the block you want to check
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[OpenGL][] Error 0x"
                  << std::hex << err << std::dec
                  << ": " << GetGLErrorString(err) << "\n";
    }
}
