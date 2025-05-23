#include "MeshData.hpp"

void MeshData::refreshTriangleColor(MeshVisMode mode, float scalar) {
    // Mesh Color
    // Data of Mesh: [ X Y Z ] [ NX NY NZ ] [ R G B ]
    {
        size_t vertCounts = m_selectedTriangles.size() * 3;
        size_t offset = vertCounts * 3 * 2 * sizeof(float); // Skip data of Pos + Norm
        size_t length = vertCounts * 3 * sizeof(float);

        std::vector<float> colorBuffer;
        colorBuffer.resize(vertCounts * 3);
        for (size_t tri_i = 0; tri_i < m_selectedTriangles.size(); ++tri_i) {
            const Triangle& tri = m_triangles[tri_i];
            HalfEdge* he = tri.he->prev;
            for (int j = 0; j < 3; ++j) {
                size_t i = tri_i * 3 + j;
                int vidx = he->vertex->index;
                switch(mode) {
                    case Normals:
                        colorBuffer[i * 3 + 0] = m_vertices[vidx].normal.x();
                        colorBuffer[i * 3 + 1] = m_vertices[vidx].normal.y();
                        colorBuffer[i * 3 + 2] = m_vertices[vidx].normal.z();
                        break;
                    case Weight:
                        colorBuffer[i * 3 + 0] = m_vertices[vidx].weight * scalar;
                        colorBuffer[i * 3 + 1] = 0;
                        colorBuffer[i * 3 + 2] = 0;
                        break;
                    default:
                        colorBuffer[i * 3 + 0] = m_meshColor.x();
                        colorBuffer[i * 3 + 1] = m_meshColor.y();
                        colorBuffer[i * 3 + 2] = m_meshColor.z();
                        break;
                }
                he = he->next;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_VBOmesh);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if(ptr) {
            memcpy(ptr, colorBuffer.data(), length);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void MeshData::refreshEdgeColor() {
    // Edges data
    // Data of Edges: [ X Y Z ] [ R G B ]
    {
        size_t vertCounts = m_edges.size() * 2;
        size_t offset = vertCounts * 3 * sizeof(float); // Skip data of Pos
        size_t length = vertCounts * 3 * sizeof(float);

        std::vector<float> colorBuffer;
        colorBuffer.resize(vertCounts * 3);
        for (size_t i = 0; i < vertCounts; ++i) {
            colorBuffer[i * 3 + 0] = m_wireframeColor.x();
            colorBuffer[i * 3 + 1] = m_wireframeColor.y();
            colorBuffer[i * 3 + 2] = m_wireframeColor.z();
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_VBOwireframe);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if(ptr) {
            memcpy(ptr, colorBuffer.data(), length);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void MeshData::changeTriangleColor(int idx, Eigen::Vector3f color) {
    size_t vertCounts = m_selectedTriangles.size() * 3;
    size_t offsetColor = vertCounts * 3 * 2 * sizeof(float); // Skip Pos + Normal

    size_t offset = offsetColor + idx * 3 * 3 * sizeof(float);
    size_t length = 3 * 3 * sizeof(float);

    std::vector<float> blueColor = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_VBOmesh);
    void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT);
    if (ptr) {
        memcpy(ptr, blueColor.data(), length);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// void MeshData::changeEdgeColor(int idx, Eigen::Vector3f color) {
// }
