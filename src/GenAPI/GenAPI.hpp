#ifndef GENAPI_HPP
#define GENAPI_HPP

#include <string>
#include <vector>
#include <map>
#include <Eigen/Dense>

// Forward declarations
class MeshData;

namespace GenAPI {

    // Structure for control point data
    struct ControlPoint {
        int id;
        std::string role;
        Eigen::Vector3f position;
        
        ControlPoint(int id, const std::string& role, const Eigen::Vector3f& pos)
            : id(id), role(role), position(pos) {}
    };

    // Structure for deformation delta
    struct DeformationDelta {
        float delta_x;
        float delta_y;
        float delta_z;
        
        DeformationDelta(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f)
            : delta_x(dx), delta_y(dy), delta_z(dz) {}
    };

    // Structure for a single animation frame
    typedef std::map<int, DeformationDelta> AnimationFrame;

    // Structure for complete animation sequence
    typedef std::vector<AnimationFrame> AnimationSequence;

    // Request structure
    struct GenerationRequest {
        std::vector<ControlPoint> control_points;
        std::string prompt;
        int length;
        
        GenerationRequest(const std::vector<ControlPoint>& cp, const std::string& p, int l)
            : control_points(cp), prompt(p), length(l) {}
    };

    // Response structure
    struct GenerationResponse {
        bool success;
        std::string error_message;
        AnimationSequence animation_frames;
        
        GenerationResponse() : success(false) {}
    };

    // Main API class
    class DeformationGenerator {
    private:
        std::string m_apiUrl;
        std::string m_tempDir;
        
        // Internal methods
        std::string constructRequestJson(const GenerationRequest& request);
        GenerationResponse parseResponseJson(const std::string& jsonResponse);
        bool performHttpRequest(const std::string& jsonData, std::string& response);
        
    public:
        DeformationGenerator(const std::string& apiUrl = "http://localhost:8080");
        ~DeformationGenerator();
        
        // Main API method
        GenerationResponse generateDeformations(const GenerationRequest& request);
        
        // Convenience methods
        GenerationResponse generatePose(const std::vector<ControlPoint>& controlPoints, 
                                      const std::string& prompt);
        GenerationResponse generateAnimation(const std::vector<ControlPoint>& controlPoints, 
                                           const std::string& prompt, int frameCount);
        
        // Utility methods
        std::vector<ControlPoint> extractControlPointsFromMesh(MeshData* meshData);
        bool storeAnimationInMesh(MeshData* meshData, const AnimationSequence& frames);
        
        // Settings
        void setApiUrl(const std::string& url) { m_apiUrl = url; }
        const std::string& getApiUrl() const { return m_apiUrl; }
        
        // Status check
        bool isApiAvailable();
    };

    // Utility functions
    std::string roleFromVertexDescription(const std::string& desc);
    Eigen::Vector3f eigenVectorFromPosition(const Eigen::Vector3d& pos);
    
} // namespace GenAPI

#endif // GENAPI_HPP