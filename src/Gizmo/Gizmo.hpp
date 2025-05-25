#ifndef GIZMO_HPP
#define GIZMO_HPP

#include <Eigen/Dense>
#include "../Visualizer/Axis.hpp"

class Gizmo{
public:
    enum AxisDir {
        None    = -1,
        AXIS_X  =  0,
        AXIS_Y  =  1,
        AXIS_Z  =  2
    };
private:
    Eigen::Vector3f m_translation;
    AxisDir m_selectedAxis;

    float m_radius;
    float m_start, m_end; // Start and end point of cone, cube, etc.

    Eigen::Vector3f* m_pointRef;
    Object::Axis* m_axis;

public:
    Gizmo(Shader* shader);
    ~Gizmo();

    void draw(const CameraParam& cameraParam);

    void setTranslation(const Eigen::Vector3f& pos) { m_translation = pos; }
    const Eigen::Vector3f& getTranslation() { return m_translation; }

    const AxisDir getSelectedAxis() { return m_selectedAxis; }
    void clearSelection() { m_selectedAxis = None; }

    AxisDir pickTranslation(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint);
    void dragAlongAxis(const Eigen::Vector3f& cam_org, const Eigen::Vector3f& nearPoint);

    void setPointRef(Eigen::Vector3f* ref);
    void unsetPointRef() { m_pointRef = nullptr; }
    void updatePointRef();
};

#endif // GIZMO_HPP
