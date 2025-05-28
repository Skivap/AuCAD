#include "MeshData.hpp"

void MeshData::computeARAP() {
    // Vertices
    Eigen::MatrixXd V(m_vertices.size(), 3);
    for (int i = 0; i < m_vertices.size(); ++i) {
        V.row(i) = m_vertices[i].originalPos.transpose();
    }
    // Indices
    Eigen::MatrixXi F(m_triangles.size(), 3);
    for (int i = 0; i < m_triangles.size(); ++i) {
        Eigen::Vector3i indices;
        HalfEdge* he = m_triangles[i].he->prev;
        for (int i = 0; i < 3; i++) {
            indices[i] = he->vertex->index;
            he = he->next;
        }
        F.row(i) = indices;
    }

    std::vector<int> constraint_indices;
    for (int i = 0; i < m_selectedVertices.size(); ++i)
        if (m_selectedVertices[i])
            constraint_indices.push_back(i);

    Eigen::VectorXi b(constraint_indices.size());
    Eigen::MatrixXd bc(constraint_indices.size(), 3);
    for (int i = 0; i < constraint_indices.size(); ++i) {
        int idx = constraint_indices[i];
        b(i) = idx;
        bc.row(i) = m_vertices[idx].pos.transpose();
    }

    igl::ARAPData arap_data;
    arap_data.with_dynamics = false;

    igl::arap_precomputation(V, F, V.cols(), b, arap_data);
    Eigen::MatrixXd V_deformed = V;
    igl::arap_solve(bc, arap_data, V_deformed);

    for (int i = 0; i < m_vertices.size(); ++i) {
        m_vertices[i].pos = V_deformed.row(i).transpose();
    }

    refreshPosition();
}
