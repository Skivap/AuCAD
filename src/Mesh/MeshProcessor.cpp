#include "MeshData.hpp"

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

void MeshData::computeARAP() {
    precomputeConstraint();
    for (int i = 0; i < m_b.size(); ++i) {
        int idx = m_b(i);
        m_bc.row(i) = m_vertices[idx].pos.transpose();  // Update handle target pos
    }

    Eigen::MatrixXd V_deformed = m_V;
    igl::arap_solve(m_bc, m_arap_data, V_deformed);

    for (int i = 0; i < m_vertices.size(); ++i)
        m_vertices[i].pos = V_deformed.row(i).transpose();

    refreshPosition();
}
