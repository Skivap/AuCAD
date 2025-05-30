#include "MeshData.hpp"

#include <iostream>
#include <limits>

void MeshData::resetSelection() {
    // Reset Selection Data
    std::fill(m_selectedEdges.begin(), m_selectedEdges.end(), false);
    std::fill(m_selectedVertices.begin(), m_selectedVertices.end(), false);
    std::fill(m_selectedTriangles.begin(), m_selectedTriangles.end(), false);

    refreshTriangleColor(MeshVisMode::None);
    refreshEdgeColor();
}

void MeshData::selectTriangle(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint) {

    Eigen::Vector3f ray_dir = (nearPoint - cam_org).normalized();

    float closest_t = std::numeric_limits<float>::max();
    int selected_triangle = -1;

    for (size_t i = 0; i < m_triangles.size(); ++i) {
        const Triangle& tri = m_triangles[i];
        Eigen::Vector3f intersectPoint;
        float t;
        bool hit = rayIntersectTriangle(cam_org, ray_dir, tri, intersectPoint, t);
        if (hit && t > 1e-6f && t < closest_t) {
            closest_t = t;
            selected_triangle = static_cast<int>(i);
        }
    }

    if (selected_triangle != -1) {
        std::cout << "Hit triangle " << selected_triangle << " at t = " << closest_t << "\n";

        m_selectedTriangles[selected_triangle] = true;
        changeTriangleColor(selected_triangle, Eigen::Vector3f(0.0f, 0.0f, 1.0f));
    } else {
        std::cout << "No triangle hit\n";
    }
}

int MeshData::selectVertex(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint) {
    Eigen::Vector3f ray_dir = (nearPoint - cam_org).normalized();

    float closest_t = std::numeric_limits<float>::max();
    int selected_triangle = -1;

    for (size_t i = 0; i < m_triangles.size(); ++i) {
        const Triangle& tri = m_triangles[i];
        Eigen::Vector3f intersectPoint;
        float t;
        bool hit = rayIntersectTriangle(cam_org, ray_dir, tri, intersectPoint, t);
        if (hit && t > 1e-6f && t < closest_t) {
            closest_t = t;
            selected_triangle = static_cast<int>(i);
        }
    }

    Eigen::Vector3f intersectPoint = cam_org + ray_dir * closest_t;
    float closest_dist = std::numeric_limits<float>::max();
    int selected_vertex = -1;
    if (selected_triangle != -1) {
        const Triangle& tri = m_triangles[selected_triangle];
        const HalfEdge* he = tri.he;
        for (int i = 0; i < 3; i++) {
            float dist = (he->vertex->pos.cast<float>() - intersectPoint).norm();
            if (dist < closest_dist) {
                closest_dist = dist;
                selected_vertex = he->vertex->index;
            }
            he = he->next;
        }

        m_selectedVertices[selected_vertex] = !m_selectedVertices[selected_vertex];
        lastSelectedVertex = selected_vertex;

    } else lastSelectedVertex = -1;

    lastSelectedVertex = m_selectedVertices[selected_vertex] ? selected_vertex : -1;
    return lastSelectedVertex;

    // precomputeConstraint();
}

bool MeshData::rayIntersectTriangle(const Eigen::Vector3f& org, const Eigen::Vector3f& dir, const Triangle& tri,
                                    Eigen::Vector3f& intersectPoint, float& t) {
    const Eigen::Vector3f& v0 = tri.he->vertex->pos.cast<float>();
    const Eigen::Vector3f& v1 = tri.he->next->vertex->pos.cast<float>();
    const Eigen::Vector3f& v2 = tri.he->prev->vertex->pos.cast<float>();

    // Möller–Trumbore intersection
    Eigen::Vector3f e1 = v1 - v0;
    Eigen::Vector3f e2 = v2 - v0;
    Eigen::Vector3f h = dir.cross(e2);
    float a = e1.dot(h);

    if (fabs(a) < 1e-6f)
        return false; // Ray is parallel to triangle

    float f = 1.0f / a;
    Eigen::Vector3f s = org - v0;
    float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Eigen::Vector3f q = s.cross(e1);
    float v = f * dir.dot(q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * e2.dot(q);
    return true;
}
