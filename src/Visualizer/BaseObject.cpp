#include "BaseObject.hpp"

Object::Base::Base(Shader* shader)
{
    this->shader = shader;
    reset();
}

Object::Base::~Base()
{

}

void Object::Base::reset()
{
    modelMatrix.setIdentity();
}

void Object::Base::translate(const Eigen::Vector3f& translation)
{
    modelMatrix(0,3) += translation.x();
    modelMatrix(1,3) += translation.y();
    modelMatrix(2,3) += translation.z();
}

void Object::Base::rotate(float angle, const Eigen::Vector3f& axis)
{
    Eigen::Matrix4f rotation;
    Eigen::Matrix3f rot = Eigen::AngleAxisf(angle, axis).matrix();

    rotation.setIdentity();
    rotation.block<3,3>(0,0) = rot;

    modelMatrix = modelMatrix * rotation;
}

void Object::Base::scale(const Eigen::Vector3f& scale)
{
    Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
    scaling(0,0) *= scale.x();
    scaling(1,1) *= scale.y();
    scaling(2,2) *= scale.z();

    modelMatrix = modelMatrix * scaling;
}
