#ifndef BASE_OBJECT_HPP
#define BASE_OBJECT_HPP

#include <glad/glad.h>
#include <Eigen/Dense>
#include <vector>

#include "../Utilities/Shader.hpp"

namespace Object {
    class Base {
    protected:
        GLuint VAO, VBO, EBO;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        Eigen::Matrix4f modelMatrix;
        Shader* shader;

    public:
        Base(Shader* _shader);
        virtual ~Base();

    public:
        virtual void init() = 0;
        virtual void draw(Eigen::Matrix4f projection, Eigen::Matrix4f view) = 0;
        virtual void cleanup() = 0;

    public: // Model Matrix Functions Transformations
        void reset();
        void translate(const Eigen::Vector3f& translation);
        void rotate(float angle, const Eigen::Vector3f& axis);
        void scale(const Eigen::Vector3f& scale);
    };

}
#endif // BASE_OBJECT_HPP
