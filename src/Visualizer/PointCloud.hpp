#ifndef POINTCLOUD_HPP
#define POINTCLOUD_HPP

#include "BaseObject.hpp"

namespace Object
{
    class PointCloud : public Base
    {
    private:
        std::vector<Eigen::Vector3f> m_offsets;
        Eigen::Vector3f m_baseColor;

    public:
        PointCloud(Shader* shader, const std::vector<Eigen::Vector3f>& vertices);
        ~PointCloud();

        void init(std::vector<float>& buffer, std::vector<unsigned int>& indices) override;
        void draw(const CameraParam& cameraParam) override;
        void draw(const CameraParam& cameraParam, const std::vector<bool>& constraint);

        inline void updateOffset(int idx, Eigen::Vector3f& val) { m_offsets[idx] = val; }
        void updateOffsets(std::vector<Eigen::Vector3f>& new_offset);
    };
}

#endif // POINTCLOUD_HPP
