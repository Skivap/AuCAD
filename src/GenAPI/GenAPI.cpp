#include "GenAPI.hpp"
#include "../Mesh/MeshData.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace GenAPI {

DeformationGenerator::DeformationGenerator(const std::string& apiUrl)
    : m_apiUrl(apiUrl)
{
    // Create temp directory for curl operations
#ifdef _WIN32
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    m_tempDir = std::string(tempPath) + "aucad_genapi\\";
    CreateDirectoryA(m_tempDir.c_str(), NULL);
#else
    m_tempDir = "/tmp/aucad_genapi/";
    system(("mkdir -p " + m_tempDir).c_str());
#endif
}

DeformationGenerator::~DeformationGenerator() {
    // Cleanup temp directory
#ifdef _WIN32
    system(("rmdir /s /q \"" + m_tempDir + "\"").c_str());
#else
    system(("rm -rf " + m_tempDir).c_str());
#endif
}

std::string DeformationGenerator::constructRequestJson(const GenerationRequest& request) {
    json requestJson;
    json controlPointsArray = json::array();

    for (const auto& cp : request.control_points) {
        json controlPoint;
        controlPoint["id"] = cp.id;
        controlPoint["role"] = cp.role;
        controlPoint["position"] = {cp.position.x(), cp.position.y(), cp.position.z()};
        controlPointsArray.push_back(controlPoint);
    }

    requestJson["control_points"] = controlPointsArray;
    requestJson["prompt"] = request.prompt;
    requestJson["length"] = request.length;

    return requestJson.dump();
}

GenerationResponse DeformationGenerator::parseResponseJson(const std::string& jsonResponse) {
    GenerationResponse response;

    try {
        json parsedJson = json::parse(jsonResponse);

        // Check if response contains error
        if (parsedJson.contains("error")) {
            response.success = false;
            response.error_message = parsedJson["error"].get<std::string>();
            return response;
        }

        // Parse animation frames array
        response.animation_frames.clear();

        if (parsedJson.is_array()) {
            // Response is a direct array of frames
            for (const auto& frameJson : parsedJson) {
                AnimationFrame frame;

                for (auto& item : frameJson.items()) {
                    const std::string& key = item.key();
                    const auto& value = item.value();
                    int vertexId = std::stoi(key);

                    if (value.contains("delta_x") && value.contains("delta_y") && value.contains("delta_z")) {
                        float deltaX = value["delta_x"].get<float>();
                        float deltaY = value["delta_y"].get<float>();
                        float deltaZ = value["delta_z"].get<float>();

                        frame[vertexId] = DeformationDelta(deltaX, deltaY, deltaZ);
                    }
                }

                response.animation_frames.push_back(frame);
            }
        } else if (parsedJson.contains("frames") && parsedJson["frames"].is_array()) {
            // Response has frames property (fallback for different API format)
            for (const auto& frameJson : parsedJson["frames"]) {
                AnimationFrame frame;

                for (auto& item : frameJson.items()) {
                    const std::string& key = item.key();
                    const auto& value = item.value();
                    int vertexId = std::stoi(key);

                    if (value.contains("delta_x") && value.contains("delta_y") && value.contains("delta_z")) {
                        float deltaX = value["delta_x"].get<float>();
                        float deltaY = value["delta_y"].get<float>();
                        float deltaZ = value["delta_z"].get<float>();

                        frame[vertexId] = DeformationDelta(deltaX, deltaY, deltaZ);
                    }
                }

                response.animation_frames.push_back(frame);
            }
        }

        response.success = !response.animation_frames.empty();

    } catch (const std::exception& e) {
        response.success = false;
        response.error_message = "JSON parsing error: " + std::string(e.what());
    }

    return response;
}

bool DeformationGenerator::performHttpRequest(const std::string& jsonData, std::string& response) {
    std::string requestFile = m_tempDir + "request.json";
    std::string responseFile = m_tempDir + "response.json";

    // Write request data to file
    std::ofstream reqFile(requestFile);
    if (!reqFile.is_open()) {
        std::cerr << "Failed to create request file: " << requestFile << std::endl;
        return false;
    }
    reqFile << jsonData;
    reqFile.close();

    // Construct curl command
    std::string curlCmd = "curl -s -X POST ";
    curlCmd += "\"" + m_apiUrl + "/generate-deformations\" ";
    curlCmd += "-H \"Content-Type: application/json\" ";
    curlCmd += "-d @\"" + requestFile + "\" ";
    curlCmd += "-o \"" + responseFile + "\"";

    // Execute curl command
    int result = system(curlCmd.c_str());

    if (result != 0) {
        std::cerr << "Curl command failed with exit code: " << result << std::endl;
        return false;
    }

    // Read response from file
    std::ifstream respFile(responseFile);
    if (!respFile.is_open()) {
        std::cerr << "Failed to read response file: " << responseFile << std::endl;
        return false;
    }

    std::ostringstream responseStream;
    responseStream << respFile.rdbuf();
    response = responseStream.str();

    std::cout << "Response: " << response << std::endl;

    respFile.close();

    return !response.empty();
}

GenerationResponse DeformationGenerator::generateDeformations(const GenerationRequest& request) {
    GenerationResponse result;

    if (request.control_points.empty()) {
        result.success = false;
        result.error_message = "No control points provided";
        return result;
    }

    if (request.prompt.empty()) {
        result.success = false;
        result.error_message = "No prompt provided";
        return result;
    }

    if (request.length <= 0) {
        result.success = false;
        result.error_message = "Invalid animation length";
        return result;
    }

    std::cout << "Generating deformations with " << request.control_points.size()
              << " control points for prompt: \"" << request.prompt << "\"" << std::endl;

    // Construct JSON request
    std::string jsonRequest = constructRequestJson(request);

    // Perform HTTP request
    std::string jsonResponse;
    bool requestSuccess = performHttpRequest(jsonRequest, jsonResponse);

    if (!requestSuccess) {
        result.success = false;
        result.error_message = "Failed to communicate with API server";
        return result;
    }

    // Parse response
    result = parseResponseJson(jsonResponse);

    if (result.success) {
        std::cout << "Successfully generated " << result.animation_frames.size()
                  << " animation frames" << std::endl;
    } else {
        std::cerr << "API request failed: " << result.error_message << std::endl;
    }

    return result;
}

GenerationResponse DeformationGenerator::generatePose(const std::vector<ControlPoint>& controlPoints,
                                                     const std::string& prompt) {
    GenerationRequest request(controlPoints, prompt, 1);
    return generateDeformations(request);
}

GenerationResponse DeformationGenerator::generateAnimation(const std::vector<ControlPoint>& controlPoints,
                                                          const std::string& prompt, int frameCount) {
    GenerationRequest request(controlPoints, prompt, frameCount);
    return generateDeformations(request);
}

std::vector<ControlPoint> DeformationGenerator::extractControlPointsFromMesh(MeshData* meshData) {
    std::vector<ControlPoint> controlPoints;

    if (!meshData) {
        return controlPoints;
    }

    const std::vector<bool>& selectedVertices = meshData->getSelectedVertices();
    const std::vector<Vertex>& vertices = meshData->getVertices();

    for (size_t i = 0; i < selectedVertices.size(); ++i) {
        if (selectedVertices[i]) {
            const Vertex& vertex = vertices[i];
            std::string role = roleFromVertexDescription(vertex.desc);
            Eigen::Vector3f position = eigenVectorFromPosition(vertex.originalPos);

            controlPoints.emplace_back(static_cast<int>(i), role, position);
        }
    }

    return controlPoints;
}

bool DeformationGenerator::storeAnimationInMesh(MeshData* meshData, const AnimationSequence& frames) {
    if (!meshData || frames.empty()) {
        return false;
    }

    // Store the animation frames in the mesh data
    meshData->storeAnimationFrames(frames);
    return true;
}

bool DeformationGenerator::isApiAvailable() {
    // Simple ping test to check if API is available
    std::string pingCmd = "curl -s --max-time 5 \"" + m_apiUrl + "\" > /dev/null 2>&1";
    int result = system(pingCmd.c_str());
    return result == 0;
}

// Utility functions
std::string roleFromVertexDescription(const std::string& desc) {
    std::string lowerDesc = desc;
    std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);

    // Common role mappings
    if (lowerDesc.find("head") != std::string::npos) return "head";
    if (lowerDesc.find("neck") != std::string::npos) return "neck";
    if (lowerDesc.find("left arm") != std::string::npos || lowerDesc.find("l arm") != std::string::npos) return "left arm";
    if (lowerDesc.find("right arm") != std::string::npos || lowerDesc.find("r arm") != std::string::npos) return "right arm";
    if (lowerDesc.find("left hand") != std::string::npos || lowerDesc.find("l hand") != std::string::npos) return "left hand";
    if (lowerDesc.find("right hand") != std::string::npos || lowerDesc.find("r hand") != std::string::npos) return "right hand";
    if (lowerDesc.find("left leg") != std::string::npos || lowerDesc.find("l leg") != std::string::npos) return "left leg";
    if (lowerDesc.find("right leg") != std::string::npos || lowerDesc.find("r leg") != std::string::npos) return "right leg";
    if (lowerDesc.find("left foot") != std::string::npos || lowerDesc.find("l foot") != std::string::npos) return "left foot";
    if (lowerDesc.find("right foot") != std::string::npos || lowerDesc.find("r foot") != std::string::npos) return "right foot";
    if (lowerDesc.find("spine") != std::string::npos) return "spine";
    if (lowerDesc.find("chest") != std::string::npos) return "chest";
    if (lowerDesc.find("pelvis") != std::string::npos) return "pelvis";

    // Default fallback
    return desc.empty() ? "unknown" : desc;
}

Eigen::Vector3f eigenVectorFromPosition(const Eigen::Vector3d& pos) {
    return Eigen::Vector3f(static_cast<float>(pos.x()),
                          static_cast<float>(pos.y()),
                          static_cast<float>(pos.z()));
}

} // namespace GenAPI
