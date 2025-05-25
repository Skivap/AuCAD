#include "MeshData.hpp"

void MeshData::computeAll() {
    computeInteriorAngle();
    computeCotangentWeight();
    computeVertexWeight();
}

void MeshData::computeInteriorAngle() {
    for(HalfEdge& he: m_halfEdges) {
        Eigen::Vector3d p0 = he.vertex->pos;
        Eigen::Vector3d p1 = he.next->vertex->pos;
        Eigen::Vector3d p2 = he.prev->vertex->pos;

        Eigen::Vector3d e1 = (p1 - p0).normalized();
        Eigen::Vector3d e2 = (p2 - p0).normalized();

        double dot = e1.dot(e2);
        dot = std::min(std::max(dot, -1.0), 1.0);
        he.angle = std::acos(dot);
    }
}

void MeshData::computeCotangentWeight() {
    for(Edge& edge: m_edges) {
        double weight = 0.0;

        auto compute_cotangent = [](const Eigen::Vector3d& v0,
                                    const Eigen::Vector3d& v1,
                                    const Eigen::Vector3d& v2) -> double {
            Eigen::Vector3d u = v1 - v0;
            Eigen::Vector3d v = v2 - v0;
            double dot = u.dot(v);
            double cross_norm = u.cross(v).norm();
            return dot / cross_norm;
        };

        if (edge.he->face) {
            Eigen::Vector3d p0 = edge.he->next->vertex->pos;
            Eigen::Vector3d p1 = edge.he->next->next->vertex->pos;
            Eigen::Vector3d p2 = edge.he->vertex->pos;
            weight += compute_cotangent(p0, p1, p2);
        }

        if (edge.he->twin && edge.he->twin->face) {
            Eigen::Vector3d p0 = edge.he->twin->next->vertex->pos;
            Eigen::Vector3d p1 = edge.he->twin->next->next->vertex->pos;
            Eigen::Vector3d p2 = edge.he->twin->vertex->pos;
            weight += compute_cotangent(p0, p1, p2);
        }

        edge.weight = 0.5 * weight;
    }
}

void MeshData::computeVertexWeight() {
    for(Vertex& v: m_vertices) {
        HalfEdge* he = v.he;
        double weights = 0;
        do {
            weights += he->edge->weight;
            he = he->next->twin;
        }
        while (he != v.he);
        v.weight = weights;
    }
}

void MeshData::computeLaplacianMatrix() {
    int n = m_vertices.size();

    L.resize(n, n);
    std::vector<Eigen::Triplet<double>> triplets;
    std::vector<double> diagonal(n, 0.0);

    for (Edge& e: m_edges) {
        int i = e.he->vertex->index;
        int j = e.he->prev->vertex->index;
        double w = e.weight;

        triplets.push_back(Eigen::Triplet<double>(i, j, -w));
        triplets.push_back(Eigen::Triplet<double>(j, i, -w));
        diagonal[i] += w;
        diagonal[j] += w;
    }

    for (int i = 0; i < n; ++i) {
        triplets.emplace_back(i, i, diagonal[i]);
    }

    L.setFromTriplets(triplets.begin(), triplets.end());
}

void computeDeformedPosition() {

}
