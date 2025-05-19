#ifndef BASE_OBJECT_HPP
#define BASE_OBJECT_HPP

#include <glad/glad.h>
#include <Eigen/Dense>
#include <vector>

#include "../Utilities/Shader.hpp"

struct CameraParam{
    const Eigen::Matrix4f& projection;
    const Eigen::Matrix4f& view;
    const Eigen::Vector3f& position;
    CameraParam(
        const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, const Eigen::Vector3f& pos)
        : projection(proj), view(view), position(pos)
    {}
};

namespace Object {
    class Base {
    protected:
        GLuint VAO, VBO, EBO;
        unsigned int bufferSize, indicesSize;

        Eigen::Matrix4f modelMatrix;
        Shader* shader;

    public:
        Base(Shader* _shader);
        virtual ~Base();

    public:
        virtual void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) = 0;
        virtual void draw(const CameraParam& cameraParam) = 0;

    public:
        // Model Matrix Functions Transformations
        void reset();
        void translate(const Eigen::Vector3f& translation);
        void rotate(float angle, const Eigen::Vector3f& axis);
        void scale(const Eigen::Vector3f& scale);

    };
}

#endif // BASE_OBJECT_HPP
