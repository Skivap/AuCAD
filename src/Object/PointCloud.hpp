#ifndef POINTCLOUD_HPP
#define POINTCLOUD_HPP

#include "BaseObject.hpp"

namespace Object
{
    class PointCloud : public Base
    {
    private:
        Eigen::Vector3f m_baseColor;
        std::vector<Eigen::Vector3f> m_offsets;

    public:
        PointCloud(Shader* shader, const std::vector<Eigen::Vector3f>& vertices);
        ~PointCloud();

        void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) override;
        void draw(const CameraParam& cameraParam) override;
    };
}

#endif // POINTCLOUD_HPP
