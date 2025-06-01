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
                        colorBuffer[i * 3 + 0] = static_cast<float>(m_vertices[vidx].normal.x());
                        colorBuffer[i * 3 + 1] = static_cast<float>(m_vertices[vidx].normal.y());
                        colorBuffer[i * 3 + 2] = static_cast<float>(m_vertices[vidx].normal.z());
                        break;
                    case Weight:
                        {
                            Eigen::Vector3d Hvec = m_vertices[vidx].meanCurvatureNormal;
                            double sign = Hvec.dot(m_vertices[vidx].normal) >= 0 ? 1.0 : -1.0;
                            double signedH = sign * Hvec.norm();

                            // Normalize by max range
                            float normalized = static_cast<float>(signedH / scalar);  // Now in [-1, 1]
                            normalized = std::min(std::max(normalized, -1.0f), 1.0f);

                            if (normalized < 0.0f) {
                                // Concave (blue to white)
                                colorBuffer[i * 3 + 0] = 1.0f + normalized;  // fades R from 1→0
                                colorBuffer[i * 3 + 1] = 1.0f + normalized;  // fades G from 1→0
                                colorBuffer[i * 3 + 2] = 1.0f;               // full blue
                            } else {
                                // Convex (white to red)
                                colorBuffer[i * 3 + 0] = 1.0f;               // full red
                                colorBuffer[i * 3 + 1] = 1.0f - normalized;  // fades G from 1→0
                                colorBuffer[i * 3 + 2] = 1.0f - normalized;  // fades B from 1→0
                            }
                            break;
                        }
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

void MeshData::changeVertexPosition(int idx, Eigen::Vector3f pos) {
    m_vertices[idx].pos = pos.cast<double>();
    {
        size_t vertCounts = m_triangles.size() * 3;
        size_t length = vertCounts * 3 * sizeof(float);

        // VBO MESH
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOmesh);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT);
        if(!ptr) return;

        Vertex& v = m_vertices[idx];

        HalfEdge* he = v.he;
        do {
            Triangle* tri = he->face;
            HalfEdge* th = tri->he->prev;
            for(int i = 0; i < 3; i++) {
                if(th->vertex->index == v.index) {
                    size_t offset = (tri->index * 3 + i) * 3 * sizeof(float);
                    memcpy((char*)ptr + offset, pos.data(), 3 * sizeof(float));
                }
                th = th->next;
            }
            he = he->next->twin;
        } while(he != v.he);

        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    {
        size_t vertCounts = m_edges.size() * 2;
        size_t length = vertCounts * 3 * sizeof(float);

        // VBO MESH
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOwireframe);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT);
        if(!ptr) return;

        Vertex& v = m_vertices[idx];

        HalfEdge* he = v.he;
        do {
            Edge* e = he->edge;
            HalfEdge* th = e->he;
            if(th->vertex->index == v.index) {
                size_t offset = (e->index * 2) * 3 * sizeof(float);
                memcpy((char*)ptr + offset, pos.data(), 3 * sizeof(float));
            }
            else {
                size_t offset = (e->index * 2 + 1) * 3 * sizeof(float);
                memcpy((char*)ptr + offset, pos.data(), 3 * sizeof(float));
            }

            he = he->next->twin;
        } while(he != v.he);

        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    m_pointCloud->updateOffset(idx, pos);
}

 void MeshData::refreshPosition() {
    {
        size_t vertCounts = m_triangles.size() * 3;
        size_t length = vertCounts * 3 * sizeof(float);

        // VBO MESH
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOmesh);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT);
        if(!ptr) return;

        std::vector<float> replacement;

        for(size_t t_idx = 0; t_idx < m_triangles.size(); t_idx++) {
            HalfEdge* he = m_triangles[t_idx].he->prev;
            for(int it = 0; it < 3; it++) {
                Vertex* v = he->vertex;
                replacement.push_back(static_cast<float>(v->pos[0]));
                replacement.push_back(static_cast<float>(v->pos[1]));
                replacement.push_back(static_cast<float>(v->pos[2]));
                he = he->next;
            }
        }

        memcpy((char*)ptr, replacement.data(), length);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    {
        size_t vertCounts = m_edges.size() * 2;
        size_t length = vertCounts * 3 * sizeof(float);

        // VBO MESH
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOwireframe);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT);
        if(!ptr) return;

        std::vector<float> replacement;

        for(int i = 0; i < m_edges.size(); i++) {
            replacement.push_back(static_cast<float>(m_edges[i].he->vertex->pos[0]));
            replacement.push_back(static_cast<float>(m_edges[i].he->vertex->pos[1]));
            replacement.push_back(static_cast<float>(m_edges[i].he->vertex->pos[2]));
            replacement.push_back(static_cast<float>(m_edges[i].he->twin->vertex->pos[0]));
            replacement.push_back(static_cast<float>(m_edges[i].he->twin->vertex->pos[1]));
            replacement.push_back(static_cast<float>(m_edges[i].he->twin->vertex->pos[2]));
        }

        memcpy((char*)ptr, replacement.data(), length);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}


void MeshData::refreshPosition(float time) {
    for (Vertex& v: m_vertices) {
        Eigen::Vector3d interpolated = v.getInterpolatedPos(time);
        Eigen::Vector3f interpolated_float = interpolated.cast<float>();
        m_pointCloud->updateOffset(v.index, interpolated_float);
        v.pos = interpolated;
    }
    refreshPosition();
}

Eigen::Vector3d Vertex::getInterpolatedPos(float time) {
    if (timeframePos.empty())
        return pos;

    auto upper = timeframePos.lower_bound(time);
    if (upper == timeframePos.begin())
        return upper->second;
    if (upper == timeframePos.end())
        return std::prev(upper)->second;

    auto lower = std::prev(upper);

    float t0 = lower->first;
    float t1 = upper->first;

    const Eigen::Vector3d& p0 = lower->second;
    const Eigen::Vector3d& p1 = upper->second;

    float alpha = (time - t0) / (t1 - t0);
    return (1 - alpha) * p0 + alpha * p1;
}
