#include "Trackball.hpp"

Trackball::Trackball() :
    m_position(0.0f, 2.0f, 2.0f), m_target(0.0f, 0.0f, 0.0f), m_up(0.0f, 1.0f, 0.0f),
    m_radius(5.0f), m_phi(0.0f), m_theta(M_PI / 2.0f),
    m_fov(45.0f), m_aspect(1.0f), m_near(0.1f), m_far(1000.0f),
    isDragging(false), m_lastX(0.0), m_lastY(0.0)
{
    updateProjMat(m_fov, m_aspect, m_near, m_far);
    updateViewMat(m_position, m_target, m_up);
}

Trackball::~Trackball() {}

void Trackball::setPerspective(float fov, float aspect, float near, float far) {
    m_fov = fov;
    m_aspect = aspect;
    m_near = near;
    m_far = far;
    updateProjMat(fov, aspect, near, far);
}

void Trackball::rotate(float deltaPhi, float deltaTheta) {
    m_phi += deltaPhi;
    m_theta += deltaTheta;

    m_theta = std::min(std::max(m_theta, 0.01f), float(M_PI - 0.01f));

    updateCameraPosition();
}

void Trackball::zoom(float delta) {
    m_radius += delta;
    m_radius = std::max(m_radius, 0.1f);
    updateCameraPosition();
}

void Trackball::pan(double x, double y) {
    Eigen::Vector3f forward = (m_target - m_position).normalized();
    Eigen::Vector3f right = forward.cross(m_up).normalized();
    Eigen::Vector3f up = right.cross(forward).normalized();

    float panSpeed = 0.01f;
    Eigen::Vector3f panOffset = x * right + y * up;
    panOffset *= panSpeed;

    m_position += panOffset;
    m_target += panOffset;

    updateViewMat(m_position, m_target, m_up);
}

void Trackball::setPosition(const Eigen::Vector3f& pos) {
    m_position = pos;
    updateViewMat(m_position, m_target, m_up);
}

void Trackball::setTarget(const Eigen::Vector3f& tgt) {
    m_target = tgt;
    updateViewMat(m_position, m_target, m_up);
}

void Trackball::startDrag(double x, double y) {
    isDragging = true;
    m_lastX = x;
    m_lastY = y;
}

void Trackball::drag(double x, double y) {
    if (!isDragging) return;

    float dx = static_cast<float>(x - m_lastX);
    float dy = static_cast<float>(y - m_lastY);

    static const float rotationSpeed = 0.01f;
    rotate(-dx * rotationSpeed, -dy * rotationSpeed);

    updateCameraPosition();

    m_lastX = x;
    m_lastY = y;
}

void Trackball::endDrag(double x, double y) {
    isDragging = false;
}

void Trackball::updateCameraPosition() {
    float x = m_radius * sin(m_theta) * cos(m_phi);
    float y = m_radius * cos(m_theta);
    float z = m_radius * sin(m_theta) * sin(m_phi);

    m_position = m_target + Eigen::Vector3f(x, y, z);
    updateViewMat(m_position, m_target, m_up);
}

void Trackball::updateViewMat(const Eigen::Vector3f& eye, const Eigen::Vector3f& org, const Eigen::Vector3f& up)
{
    Eigen::Vector3f zaxis = (eye - org).normalized();
    Eigen::Vector3f xaxis = up.cross(zaxis).normalized();
    Eigen::Vector3f yaxis = zaxis.cross(xaxis);

    m_viewMat <<
        xaxis.x(), xaxis.y(), xaxis.z(), -eye.dot(xaxis),
        yaxis.x(), yaxis.y(), yaxis.z(), -eye.dot(yaxis),
        zaxis.x(), zaxis.y(), zaxis.z(), -eye.dot(zaxis),
        0, 0, 0, 1;
}

void Trackball::updateProjMat(float fov, float aspect, float near, float far)
{
    float tanHalfFovy = tan(fov / 2.0f);

    m_projMat = Eigen::Matrix4f::Zero();
    m_projMat(0,0) = 1.0f / (aspect * tanHalfFovy);
    m_projMat(1,1) = 1.0f / tanHalfFovy;
    m_projMat(2,2) = -(far + near) / (far - near);
    m_projMat(2,3) = -(2.0f * far * near) / (far - near);
    m_projMat(3,2) = -1.0f;
}
