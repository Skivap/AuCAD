#include "Gizmo.hpp"
#include <iostream>

Gizmo::Gizmo(Shader* shader) {
    float rad = 0.05f;
    float rad_scale = 4.0f;
    // Overall Scale
    float scale = 0.2f;
    // Base & Tip
    float start = 0.7f;
    int segments = 16;

    m_radius = rad;
    m_start = start * scale;
    m_end = 1.0f * scale;
    m_selectedAxis = None;

    m_axis = new Object::Axis(shader, rad, rad_scale, scale, start, segments);

    m_translation = Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    m_axis->setTranslation(m_translation);
}

Gizmo::~Gizmo() {

}

void Gizmo::draw(const CameraParam& cameraParam) {
    m_axis->draw(cameraParam);
}

Gizmo::AxisDir Gizmo::pickTranslation(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint) {

    const Eigen::Vector3f ray_dir = (nearPoint - cam_org).normalized();

    const Eigen::Vector3f axisDirs[3] = {
        Eigen::Vector3f(1, 0, 0),
        Eigen::Vector3f(0, 1, 0),
        Eigen::Vector3f(0, 0, 1)
    };

    AxisDir closestAxis = None;
    float minDistance = std::numeric_limits<float>::max();


    for (int i = 0; i < 3; ++i) {
        const Eigen::Vector3f& axis = axisDirs[i];

        // Define cone
        Eigen::Vector3f base = m_translation + m_start * axis;
        Eigen::Vector3f tip  = m_translation + m_end   * axis;
        Eigen::Vector3f axisDir = (base - tip).normalized();

        float h = (base - tip).norm();
        float r = m_radius;

        float cos2 = (h * h) / (h * h + r * r);

        // Ray-cone intersection test
        Eigen::Vector3f co = cam_org - tip;
        float dv = ray_dir.dot(axisDir);
        float cv = co.dot(axisDir);

        float A = dv * dv - cos2 * ray_dir.dot(ray_dir);
        float B = 2.0f * (dv * cv - cos2 * ray_dir.dot(co));
        float C = cv * cv - cos2 * co.dot(co);

        float discriminant = B * B - 4.0f * A * C;
        if (discriminant < 0.0f || std::abs(A) < 1e-6f) {
            continue; // No hit or degenerate
        }

        float sqrtDisc = std::sqrt(discriminant);
        float t0 = (-B - sqrtDisc) / (2.0f * A);
        float t1 = (-B + sqrtDisc) / (2.0f * A);

        float t = (t0 > 0.0f) ? t0 : ((t1 > 0.0f) ? t1 : -1.0f);
        if (t < 0.0f){
            continue; // No valid forward hit
        } else {
            Eigen::Vector3f p = cam_org + t * ray_dir;
            float along = (p - tip).dot(axisDir);
            if (along >= 0.0f && along <= h) {
                if (t < minDistance) {
                    minDistance = t;
                    closestAxis = static_cast<AxisDir>(i);
                }
            }
        }

        float denom = ray_dir.dot(axisDir);
        if (std::abs(denom) > 1e-6f) { // not parallel
            float t_disk = (base - cam_org).dot(axisDir) / denom;
            if (t_disk > 0.0f && t_disk < minDistance) {
                Eigen::Vector3f hitPoint = cam_org + t_disk * ray_dir;
                if ((hitPoint - base).squaredNorm() <= r * r) {
                    minDistance = t_disk;
                    closestAxis = static_cast<AxisDir>(i);
                }
            }
        }
    }

    std::cout << "Closest Axis (Cone): " << (int)closestAxis << "\n";
    m_selectedAxis = closestAxis;
    return closestAxis;
}

void Gizmo::dragAlongAxis(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint) {
    if (m_selectedAxis == None) return;

    Eigen::Vector3f ray_dir = (nearPoint - cam_org).normalized();

    Eigen::Vector3f axisDir;
    switch (m_selectedAxis) {
        case AXIS_X: axisDir = Eigen::Vector3f(1, 0, 0); break;
        case AXIS_Y: axisDir = Eigen::Vector3f(0, 1, 0); break;
        case AXIS_Z: axisDir = Eigen::Vector3f(0, 0, 1); break;
        default: return;
    }

    Eigen::Vector3f p1 = cam_org;
    Eigen::Vector3f d1 = ray_dir;
    Eigen::Vector3f p2 = m_translation;
    Eigen::Vector3f d2 = axisDir;

    Eigen::Vector3f r = p1 - p2;
    float a = d1.dot(d1);
    float b = d1.dot(d2);
    float c = d2.dot(d2);
    float d = d1.dot(r);
    float e = d2.dot(r);

    float denom = a * c - b * b;
    if (std::abs(denom) < 1e-6f) return;

    float s = (b * e - c * d) / denom;

    // Closest point on the ray to the axis
    Eigen::Vector3f closestOnRay = p1 + s * d1;

    // Project this point onto the axis
    Eigen::Vector3f vecToPoint = closestOnRay - m_translation;
    float axisDistance = vecToPoint.dot(axisDir) - (m_start + (m_end - m_start) / 2);

    // Update translation along axis
    m_translation += axisDistance * axisDir;
    m_axis->setTranslation(m_translation);
}
