#include "MeshData.hpp"

#include <iostream>
#include <limits>

void MeshData::resetSelection() {
    // Reset Selection Data
    std::fill(m_selectedEdges.begin(), m_selectedEdges.end(), false);
    std::fill(m_selectedVertices.begin(), m_selectedVertices.end(), false);
    std::fill(m_selectedTriangles.begin(), m_selectedTriangles.end(), false);

    // Mesh Color
    // Data of Mesh: [ X Y Z ] [ NX NY NZ ] [ R G B ]
    {
        size_t vertCounts = m_selectedTriangles.size() * 3;
        size_t offset = vertCounts * 3 * 2 * sizeof(float); // Skip data of Pos + Norm
        size_t length = vertCounts * 3 * sizeof(float);

        std::vector<float> colorBuffer;
        colorBuffer.resize(vertCounts * 3);
        for (size_t i = 0; i < vertCounts; ++i) {
            colorBuffer[i * 3 + 0] = m_meshColor.x();
            colorBuffer[i * 3 + 1] = m_meshColor.y();
            colorBuffer[i * 3 + 2] = m_meshColor.z();
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_VBOmesh);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, offset, length, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if(ptr) {
            memcpy(ptr, colorBuffer.data(), length);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

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

void MeshData::selectTriangle(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& point) {
    Eigen::Vector3f ray_dir = (point - cam_org).normalized();

       float closest_t = std::numeric_limits<float>::max();
       int selected_triangle = -1;

       for (size_t i = 0; i < m_triangles.size(); ++i) {
           const Triangle& tri = m_triangles[i];
           const Eigen::Vector3f& v0 = tri.he->vertex->pos;
           const Eigen::Vector3f& v1 = tri.he->next->vertex->pos;
           const Eigen::Vector3f& v2 = tri.he->prev->vertex->pos;

           // Möller–Trumbore intersection
           Eigen::Vector3f e1 = v1 - v0;
           Eigen::Vector3f e2 = v2 - v0;
           Eigen::Vector3f h = ray_dir.cross(e2);
           float a = e1.dot(h);

           if (fabs(a) < 1e-6f)
               continue; // Ray is parallel to triangle

           float f = 1.0f / a;
           Eigen::Vector3f s = cam_org - v0;
           float u = f * s.dot(h);
           if (u < 0.0f || u > 1.0f)
               continue;

           Eigen::Vector3f q = s.cross(e1);
           float v = f * ray_dir.dot(q);
           if (v < 0.0f || u + v > 1.0f)
               continue;

           float t = f * e2.dot(q);
           if (t > 1e-6f && t < closest_t) {
               closest_t = t;
               selected_triangle = static_cast<int>(i);
           }
       }

       if (selected_triangle != -1) {
           std::cout << "Hit triangle " << selected_triangle << " at t = " << closest_t << "\n";

           m_selectedTriangles[selected_triangle] = true;

           size_t vertCounts = m_selectedTriangles.size() * 3;
           size_t offsetColor = vertCounts * 3 * 2 * sizeof(float); // Skip Pos + Normal

           size_t offset = offsetColor + selected_triangle * 3 * 3 * sizeof(float);
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
       } else {
           std::cout << "No triangle hit\n";
       }
}
