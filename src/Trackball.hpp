#ifndef TRACKBALL_HPP
#define TRACKBALL_HPP

#include <Eigen/Dense>

class Trackball {
private:
    // Matrix
    Eigen::Matrix4f m_projMat;
    Eigen::Matrix4f m_viewMat;

    // View Matrix
    Eigen::Vector3f m_position;
    Eigen::Vector3f m_target;
    Eigen::Vector3f m_up;

    // Rotation View
    float m_radius;
    float m_phi;
    float m_theta;

    // Projection
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    // Drag
    bool isDragging;
    double m_lastX;
    double m_lastY;

public:
    Trackball(int width, int height);
    ~Trackball();

    const Eigen::Matrix4f& getProjectionMatrix() const { return m_projMat; }
    const Eigen::Matrix4f& getViewMatrix() const { return m_viewMat; }
    const Eigen::Vector3f& getPosition() const { return m_position; }

    void setPerspective(float fov, float aspect, float near, float far);
    void resize(int width, int height);

    void rotate(float deltaPhi, float deltaTheta);
    void zoom(float delta);
    void pan(double x, double y);

    void setPosition(const Eigen::Vector3f& pos);
    void setTarget(const Eigen::Vector3f& tgt);

public:
    void startDrag(double x, double y);
    void drag(double x, double y);
    void endDrag(double x, double y);


private:
    void updateCameraPosition();
    void updateViewMat(const Eigen::Vector3f& eye, const Eigen::Vector3f& org, const Eigen::Vector3f& up);
    void updateProjMat(float fov, float aspect, float near, float far);
};

#endif // TRACKBALL_HPP
