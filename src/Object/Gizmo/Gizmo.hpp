#ifndef GIZMO_HPP
#define GIZMO_HPP

#include <Eigen/Dense>

class Gizmo{
public:
    enum Axis {
        None    = -1,
        AXIS_X  =  0,
        AXIS_Y  =  1,
        AXIS_Z  =  2
    };
private:
    Eigen::Vector3f m_translation;
    Axis m_selectedAxis;

    float m_radius;
    float m_start, m_end; // Start and end point of cone, cube, etc.

public:
    Gizmo(float rad, float start, float end);
    ~Gizmo();

    void setTranslation(const Eigen::Vector3f& pos) { m_translation = pos; }
    const Eigen::Vector3f& getTranslation() { return m_translation; }

    const Axis getSelectedAxis() { return m_selectedAxis; }
    void clearSelection() { m_selectedAxis = None; }

    Axis pickTranslation(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint);
    void dragAlongAxis(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint);
};

#endif // GIZMO_HPP
