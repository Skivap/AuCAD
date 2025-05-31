#include "MeshData.hpp"
#include "../GenAPI/GenAPI.hpp"

void MeshData::precomputeARAP() {
    m_V.resize(m_vertices.size(), 3);
    for (int i = 0; i < m_vertices.size(); ++i)
        m_V.row(i) = m_vertices[i].originalPos.transpose();

    m_F.resize(m_triangles.size(), 3);
    for (int i = 0; i < m_triangles.size(); ++i) {
        Eigen::Vector3i indices;
        HalfEdge* he = m_triangles[i].he->prev;
        for (int j = 0; j < 3; ++j) {
            indices[j] = he->vertex->index;
            he = he->next;
        }
        m_F.row(i) = indices;
    }
}

void MeshData::precomputeConstraint() {
    std::vector<int> constraint_indices;
    for (int i = 0; i < m_selectedVertices.size(); ++i)
        if (m_selectedVertices[i])
            constraint_indices.push_back(i);

    m_b.resize(constraint_indices.size());
    m_bc.resize(constraint_indices.size(), 3);

    for (int i = 0; i < constraint_indices.size(); ++i) {
        int idx = constraint_indices[i];
        m_b(i) = idx;
        m_bc.row(i) = m_vertices[idx].pos.transpose();  // Use current handle positions
    }

    m_arap_data.with_dynamics = false;
    igl::arap_precomputation(m_V, m_F, m_V.cols(), m_b, m_arap_data);
}

void MeshData::saveTimeFrame(float time) {
    for(Vertex& v: m_vertices) {
        v.timeframePos[time] = v.pos;
    }
}

void MeshData::computeARAP() {
    std::cout << "1111" << std::endl;
    precomputeConstraint();
    std::cout << "2222" << std::endl;
    for (int i = 0; i < m_b.size(); ++i) {
        int idx = m_b(i);
        m_bc.row(i) = m_vertices[idx].pos.transpose();  // Update handle target pos
    }
    std::cout << "3333" << std::endl;
    Eigen::MatrixXd V_deformed = m_V;
    igl::arap_solve(m_bc, m_arap_data, V_deformed);

    std::cout << "4444" << std::endl;
    for (int i = 0; i < m_vertices.size(); ++i)
        m_vertices[i].pos = V_deformed.row(i).transpose();

    std::cout << "5555" << std::endl;
    // refreshPosition();
    std::cout << "6666" << std::endl;
}

void MeshData::storeAnimationFrames(const GenAPI::AnimationSequence& frames) {
    std::cout << "Storing " << frames.size() << " animation frames..." << std::endl;

    // Store base positions before animation
    m_basePositions.clear();
    for (int i = 0; i < m_vertices.size(); ++i) {
        m_basePositions[i] = m_vertices[i].pos;
    }
    std::cout << "Stored " << m_basePositions.size() << " base positions" << std::endl;

    // Store animation frames
    m_storedAnimationFrames = frames;

    // Apply frames to timeframePos at integer time values
    for (int frameIndex = 0; frameIndex < frames.size(); ++frameIndex) {
        float timeframe = static_cast<float>(frameIndex + 1); // 1.0, 2.0, 3.0, etc.
        std::cout << "Processing frame " << frameIndex << " for timeframe " << timeframe << std::endl;

        int verticesWithDeltas = 0;
        // Apply this frame's deltas to base positions
        for (int i = 0; i < m_vertices.size(); ++i) {
            Eigen::Vector3d newPos = m_basePositions[i]; // Start with base position

            // Apply delta if this vertex is in this frame
            const GenAPI::AnimationFrame& frame = frames[frameIndex];
            auto it = frame.find(i);
            if (it != frame.end()) {
                const GenAPI::DeformationDelta& delta = it->second;
                newPos += Eigen::Vector3d(delta.delta_x, delta.delta_y, delta.delta_z);
                verticesWithDeltas++;
                std::cout << "  Vertex " << i << " delta: (" << delta.delta_x << ", " << delta.delta_y << ", " << delta.delta_z << ")" << std::endl;
            }

            // Store in vertex timeframe
            m_vertices[i].timeframePos[timeframe] = newPos;
            m_vertices[i].pos = newPos;
        }
        std::cout << "AAAA" << std::endl;
        computeARAP();
        std::cout << "BBBB" << std::endl;
        saveTimeFrame(timeframe);
        std::cout << "Frame " << frameIndex << " applied deltas to " << verticesWithDeltas << " vertices" << std::endl;
    }

    // Save initial frame at time 0
    saveTimeFrame(0.0f);
    std::cout << "Animation frames stored successfully!" << std::endl;
}

void MeshData::applyAnimationFrame(int frameIndex) {
    if (frameIndex < 0 || frameIndex >= m_storedAnimationFrames.size()) {
        return;
    }

    const GenAPI::AnimationFrame& frame = m_storedAnimationFrames[frameIndex];

    // Apply deltas to base positions
    for (const auto& pair : frame) {
        int vertexId = pair.first;
        const GenAPI::DeformationDelta& delta = pair.second;

        if (vertexId >= 0 && vertexId < m_vertices.size()) {
            Eigen::Vector3d basePos = m_basePositions[vertexId];
            Eigen::Vector3d newPos = basePos + Eigen::Vector3d(delta.delta_x, delta.delta_y, delta.delta_z);
            m_vertices[vertexId].pos = newPos;
        }
    }

    refreshPosition();
}

void MeshData::clearAnimationFrames() {
    m_storedAnimationFrames.clear();
    m_basePositions.clear();

    // Clear timeframe positions except for time 0
    for (Vertex& vertex : m_vertices) {
        auto it = vertex.timeframePos.begin();
        while (it != vertex.timeframePos.end()) {
            if (it->first > 0.0f) {
                it = vertex.timeframePos.erase(it);
            } else {
                ++it;
            }
        }
    }
}

bool MeshData::hasAnimationFrames() const {
    return !m_storedAnimationFrames.empty();
}

int MeshData::getAnimationFrameCount() const {
    return static_cast<int>(m_storedAnimationFrames.size());
}
