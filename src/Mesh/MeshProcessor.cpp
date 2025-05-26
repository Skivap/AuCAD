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
    // Compute from the current pos, not original
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

void MeshData::computeDeformedPosition() {
    int n = m_vertices.size();

    // We want to solve Lp' = b
    // - L is the laplacian matrix
    // - b is where vertix i and neighbor ring j:
    //      = sum(w_ij / 2 * (Ri + Rj) (pi - pj))

    // Setup Laplacian Matrix
    Eigen::SparseMatrix<double> L(n, n);
    std::vector<Eigen::Triplet<double> > triplets;
    std::vector<double> diag(n, 0.0);

    for(const Edge& e : m_edges){
        int i = e.he->vertex->index;
        int j = e.he->prev->vertex->index;
        if(m_selectedVertices[i] || m_selectedVertices[j])
            continue;

        double w = e.weight;
        triplets.emplace_back(i,j, -w);
        triplets.emplace_back(j,i, -w);
        diag[i] += w;  diag[j] += w;
    }
    for(int i = 0; i < n; i++){
        if(m_selectedVertices[i]){
            triplets.emplace_back(i, i, 1.0);
        } else {
            triplets.emplace_back(i, i, diag[i]);
        }
    }

    L.setFromTriplets(triplets.begin(), triplets.end());

    // Setup B
    Eigen::MatrixXd B(n, 3);
    B.setZero();
    for(int i = 0; i < n; i++){
        if(m_selectedVertices[i]){
            B.row(i) = m_vertices[i].originalPos.transpose();
        } else {
            const Eigen::Vector3d& pi = m_vertices[i].originalPos;
            const Eigen::Matrix3d Ri = m_vertices[i].deformedRot.block<3, 3>(0, 0); // Convert from 4x4 to 3x3

            HalfEdge* he = m_vertices[i].he;
            do {
                Vertex* vj = he->twin->vertex;
                int j = vj->index;
                const Eigen::Vector3d& pj = vj->originalPos;
                const Eigen::Matrix3d Rj = vj->deformedRot.block<3, 3>(0, 0);

                double w = he->edge->weight;
                B.row(i) += 0.5 * w * ((Ri + Rj) * (pi - pj)).transpose();

                he = he->next->twin;
            } while (he != m_vertices[i].he);
        }
    }

    Eigen::SimplicialLLT<decltype(L)> solver;
    solver.compute(L);
    if(solver.info()!=Eigen::Success){
        std::cerr<<"Decomposition failed\n";
        return;
    }
    Eigen::MatrixXd newPos(n,3);
    for(int d=0;d<3;++d)
        newPos.col(d) = solver.solve(B.col(d));

    for(int i=0;i<n;++i)
        m_vertices[i].pos = newPos.row(i).transpose();

    refreshPosition();
}

void MeshData::computeLaplacianSurfaceModeling() {
    int n = m_vertices.size();

    // (1) Setup Laplacian Matrix
    Eigen::SparseMatrix<double> L(n, n);
    std::vector<Eigen::Triplet<double> > triplets;
    std::vector<double> diag(n, 0.0);

    for (const Edge& e : m_edges){
        int i = e.he->vertex->index;
        int j = e.he->prev->vertex->index;
        // if(m_selectedVertices[i] || m_selectedVertices[j])
        //     continue;

        double w = e.weight;
        triplets.emplace_back(i,j, -w);
        triplets.emplace_back(j,i, -w);
        diag[i] += w;  diag[j] += w;
    }

    for (int i = 0; i < n; i++){
        if(m_selectedVertices[i]){
            triplets.emplace_back(i, i, 1.0);
        } else {
            triplets.emplace_back(i, i, diag[i]);
        }
    }

    L.setFromTriplets(triplets.begin(), triplets.end());

    // (2) Setup Delta
    Eigen::MatrixXd delta(n, 3);
    for (int i = 0; i < n; i++) {
        if (m_selectedVertices[i]) {
            delta.row(i) = m_vertices[i].pos;
        }
        else {
            Eigen::Vector3d weights(0.0, 0.0, 0.0);

            Vertex& p_i = m_vertices[i];
            HalfEdge* he = p_i.he;
            do {
                Vertex* p_j = he->twin->vertex;
                weights += he->edge->weight * (p_i.originalPos - p_j->originalPos);
                he = he->next->twin;
            } while (he != p_i.he);

            delta.row(i) = weights;
        }
    }

    // (3) Solve Lp' = δ
    Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> solver;
    solver.compute(L);
    if (solver.info() != Eigen::Success) {
        std::cerr << "Decomposition failed!" << std::endl;
        return;
    }

    Eigen::MatrixXd newPos(n, 3);
    for (int dim = 0; dim < 3; ++dim) {
        Eigen::VectorXd rhs = delta.col(dim);
        Eigen::VectorXd sol = solver.solve(rhs);
        if (solver.info() != Eigen::Success) {
            std::cerr << "Solving failed in dimension " << dim << std::endl;
            return;
        }
        newPos.col(dim) = sol;
    }

    for(int i = 0; i < m_vertices.size(); i++) {
        m_vertices[i].pos = newPos.row(i).transpose();
    }

    refreshPosition();
}
