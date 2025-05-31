#include "Interface.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Mesh/MeshData.hpp"
#include "GenAPI/GenAPI.hpp"
#include <iostream>
#include <thread>

Interface::Interface(GLFWwindow* window, int screen_width, int screen_height)
    : m_window(window), m_width(screen_width), m_height(screen_height), m_computeDeformedPos(false), safeTimeframe(false), m_weightThreshold(0.1f),
      doRefresh(false), timestep(0.0f), m_meshData(nullptr), m_showVertexPanel(true),
      m_generator(std::make_unique<GenAPI::DeformationGenerator>()), m_showGenerationPanel(true),
      m_animationLength(1), m_apiUrl("http://localhost:8080"), m_isGenerating(false), m_apiConnected(false),
      m_processingAllFrames(false), m_currentProcessingFrame(0), m_totalFramesToProcess(11)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize generation panel
    memset(m_promptBuffer, 0, sizeof(m_promptBuffer));
    strcpy(m_promptBuffer, "make the character wave");

    // Check API connection in background
    std::thread([this]() {
        checkApiConnection();
    }).detach();
}

Interface::~Interface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Interface::resize(int width, int height) {
    m_width = width;
    m_height = height;
}

void Interface::draw() {
    m_computeDeformedPos = false;
    safeTimeframe = false;
    doRefresh = false;

    // Handle ARAP all frames processing
    if (m_processingAllFrames) {
        if (m_currentProcessingFrame < m_totalFramesToProcess) {
            timestep = static_cast<float>(m_currentProcessingFrame);
            m_computeDeformedPos = true;
            safeTimeframe = true;
            doRefresh = true;
            m_currentProcessingFrame++;
        } else {
            // Finished processing all frames
            m_processingAllFrames = false;
            m_currentProcessingFrame = 0;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Position at the bottom
    float windowHeight = 200.0f;
    ImGui::SetNextWindowPos(ImVec2(0, m_height - windowHeight));
    ImGui::SetNextWindowSize(ImVec2(m_width, windowHeight));

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Text("Selection Mode:");
    if (ImGui::RadioButton("Vertex", m_selectionMode == SelectionMode::Vertex)) {
        m_selectionMode = SelectionMode::Vertex;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Axis", m_selectionMode == SelectionMode::AxisDebug)) {
        m_selectionMode = SelectionMode::AxisDebug;
    }
    ImGui::SameLine();
    ImGui::InputText("Description", buffer, 64);

    ImGui::Separator();

    ImGui::Text("Visualization:");
    if (ImGui::Button("Default")) m_visualizeMode = 1;
    ImGui::SameLine();
    if (ImGui::Button("Normal")) m_visualizeMode = 2;
    ImGui::SameLine();
    if (ImGui::Button("Compute ARAP")) m_computeDeformedPos = true;
    ImGui::SameLine();
    if (ImGui::Button("Save Timeframe")) safeTimeframe = true;
    ImGui::SameLine();
    if (ImGui::Button("ARAP all frames")) {
        // Start processing all full frames from 0.0 to 10.0
        m_processingAllFrames = true;
        m_currentProcessingFrame = 0;
        m_totalFramesToProcess = 11; // 0, 1, 2, ..., 10
    }
    
    // Show progress if processing
    if (m_processingAllFrames) {
        ImGui::SameLine();
        ImGui::Text("Processing frame %d/%d", m_currentProcessingFrame, m_totalFramesToProcess);
    }

    if (ImGui::SliderFloat("Timestep", &timestep, 0.0f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp)) {
        doRefresh = true;
    }

    ImGui::Separator();

    ImGui::Text("Panels:");
    ImGui::Checkbox("Show Vertex Panel", &m_showVertexPanel);
    ImGui::SameLine();
    ImGui::Checkbox("Show Generation Panel", &m_showGenerationPanel);

    ImGui::End();

    // Draw vertex panel
    drawVertexPanel();

    // Draw generation panel
    drawGenerationPanel();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::drawVertexPanel() {
    if (!m_showVertexPanel || !m_meshData) return;

    // Position the vertex panel on the right side
    float panelWidth = 350.0f;
    ImGui::SetNextWindowPos(ImVec2(m_width - panelWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, m_height - 200.0f)); // Leave space for the controls panel

    ImGui::Begin("Selected Vertices", &m_showVertexPanel, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    const std::vector<bool>& selectedVertices = m_meshData->getSelectedVertices();
    const std::vector<class Vertex>& vertices = m_meshData->getVertices();

    // Count selected vertices
    int selectedCount = 0;
    for (bool selected : selectedVertices) {
        if (selected) selectedCount++;
    }

    ImGui::Text("Selected Vertices: %d", selectedCount);
    ImGui::Separator();

    if (selectedCount == 0) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No vertices selected");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Right-click on vertices to select them");
        ImGui::End();
        return;
    }

    // Track if any position was modified
    bool positionModified = false;

    for (int i = 0; i < selectedVertices.size(); ++i) {
        if (selectedVertices[i]) {
            const class Vertex& vertex = vertices[i];

            // Create a unique ID for this vertex
            ImGui::PushID(i);

            // Header for this vertex
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            ImGui::Text("Vertex ID: %d", i);
            ImGui::PopStyleColor();

            // Deselect button
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            if (ImGui::SmallButton("Deselect")) {
                m_meshData->deselectVertex(i);
            }
            ImGui::PopStyleColor(3);

            // Editable label/description
            static char labelBuffer[64];
            strncpy(labelBuffer, vertex.desc, 63);
            labelBuffer[63] = '\0';

            ImGui::Text("Label:");
            ImGui::SameLine();
            if (ImGui::InputText("##label", labelBuffer, 64)) {
                // Update vertex description
                class Vertex& mutableVertex = m_meshData->getVertex(i);
                strncpy(mutableVertex.desc, labelBuffer, 63);
                mutableVertex.desc[63] = '\0';
            }

            // Position input fields with labels
            float pos[3] = {
                static_cast<float>(vertex.pos.x()),
                static_cast<float>(vertex.pos.y()),
                static_cast<float>(vertex.pos.z())
            };

            ImGui::Text("Position:");
            ImGui::Columns(3, "xyz_columns", false);

            ImGui::Text("X:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat("##x", &pos[0], 0.0f, 0.0f, "%.3f")) {
                Eigen::Vector3f newPos(pos[0], pos[1], pos[2]);
                m_meshData->changeVertexPosition(i, newPos);
                positionModified = true;
            }

            ImGui::NextColumn();
            ImGui::Text("Y:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat("##y", &pos[1], 0.0f, 0.0f, "%.3f")) {
                Eigen::Vector3f newPos(pos[0], pos[1], pos[2]);
                m_meshData->changeVertexPosition(i, newPos);
                positionModified = true;
            }

            ImGui::NextColumn();
            ImGui::Text("Z:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputFloat("##z", &pos[2], 0.0f, 0.0f, "%.3f")) {
                Eigen::Vector3f newPos(pos[0], pos[1], pos[2]);
                m_meshData->changeVertexPosition(i, newPos);
                positionModified = true;
            }

            ImGui::Columns(1);

            // Add some spacing between vertices
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void Interface::drawGenerationPanel() {
    if (!m_showGenerationPanel) return;

    // Position the generation panel on the left side
    float panelWidth = 400.0f;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, m_height - 200.0f)); // Leave space for controls panel

    ImGui::Begin("Pose/Animation Generation", &m_showGenerationPanel, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // API Status
    ImGui::Text("API Status:");
    ImGui::SameLine();
    if (m_apiConnected) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Refresh")) {
        std::thread([this]() {
            checkApiConnection();
        }).detach();
    }

    ImGui::Separator();

    // API URL Configuration
    static char urlBuffer[256];
    if (strlen(urlBuffer) == 0) {
        strcpy(urlBuffer, m_apiUrl.c_str());
    }

    ImGui::Text("API URL:");
    if (ImGui::InputText("##api_url", urlBuffer, sizeof(urlBuffer))) {
        m_apiUrl = std::string(urlBuffer);
        m_generator->setApiUrl(m_apiUrl);
        // Check connection when URL changes
        std::thread([this]() {
            checkApiConnection();
        }).detach();
    }

    ImGui::Separator();

    // Control Points Info
    if (m_meshData) {
        const std::vector<bool>& selectedVertices = m_meshData->getSelectedVertices();
        int selectedCount = 0;
        for (bool selected : selectedVertices) {
            if (selected) selectedCount++;
        }

        ImGui::Text("Control Points: %d", selectedCount);

        if (selectedCount == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "No vertices selected");
            ImGui::TextWrapped("Select vertices to use as control points for generation");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Ready for generation");
        }
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No mesh loaded");
    }

    ImGui::Separator();

    // Prompt Input
    ImGui::Text("Animation Prompt:");
    ImGui::InputTextMultiline("##prompt", m_promptBuffer, sizeof(m_promptBuffer), ImVec2(-1, 60));

    // Animation Length
    ImGui::Text("Animation Length:");
    ImGui::SliderInt("##length", &m_animationLength, 1, 20, "%d frames");

    if (m_animationLength == 1) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Static Pose)");
    } else {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Animation)");
    }

    ImGui::Separator();

    // Animation Status
    if (m_meshData && m_meshData->hasAnimationFrames()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Animation Loaded:");
        ImGui::Text("Frames: %d", m_meshData->getAnimationFrameCount());
        ImGui::TextWrapped("Use timeframe slider (1.0, 2.0, 3.0...) to view animation");

        if (ImGui::Button("Clear Animation", ImVec2(-1, 25))) {
            m_meshData->clearAnimationFrames();
        }

        ImGui::Separator();
    }

    // Generation Buttons
    bool canGenerate = m_meshData && m_apiConnected && !m_isGenerating && strlen(m_promptBuffer) > 0;

    if (!canGenerate) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button("Generate Pose/Animation", ImVec2(-1, 40))) {
        if (canGenerate) {
            generateDeformations();
        }
    }

    if (!canGenerate) {
        ImGui::PopStyleVar();
    }

    // Generation Status
    if (m_isGenerating) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Generating...");
        ImGui::ProgressBar(-1.0f * ImGui::GetTime(), ImVec2(-1, 0), "");
    } else if (!m_lastError.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error:");
        ImGui::TextWrapped("%s", m_lastError.c_str());
        if (ImGui::Button("Clear Error")) {
            m_lastError.clear();
        }
    }

    ImGui::Separator();

    // Quick Preset Prompts
    ImGui::Text("Quick Presets:");

    if (ImGui::Button("Wave", ImVec2(60, 25))) {
        strcpy(m_promptBuffer, "make the character wave");
        m_animationLength = 6;
    }
    ImGui::SameLine();
    if (ImGui::Button("Jump", ImVec2(60, 25))) {
        strcpy(m_promptBuffer, "character jumps with arms up");
        m_animationLength = 8;
    }
    ImGui::SameLine();
    if (ImGui::Button("Bend", ImVec2(60, 25))) {
        strcpy(m_promptBuffer, "bend forward to pick something up");
        m_animationLength = 1;
    }

    if (ImGui::Button("Walk", ImVec2(60, 25))) {
        strcpy(m_promptBuffer, "make the character walk naturally forward");
        m_animationLength = 12;
    }
    ImGui::SameLine();
    if (ImGui::Button("Dance", ImVec2(60, 25))) {
        strcpy(m_promptBuffer, "character does a simple dance");
        m_animationLength = 16;
    }
    ImGui::SameLine();
    if (ImGui::Button("Salute", ImVec2(60, 25))) {
        strcpy(m_promptBuffer, "character salutes with right hand");
        m_animationLength = 1;
    }

    ImGui::End();
}

void Interface::checkApiConnection() {
    m_apiConnected = m_generator->isApiAvailable();
}

void Interface::generateDeformations() {
    if (!m_meshData || m_isGenerating) {
        return;
    }

    m_isGenerating = true;
    m_lastError.clear();

    // Run generation in background thread
    std::thread([this]() {
        try {
            // Extract control points from selected vertices
            std::vector<GenAPI::ControlPoint> controlPoints = m_generator->extractControlPointsFromMesh(m_meshData);

            if (controlPoints.empty()) {
                m_lastError = "No control points available. Please select vertices first.";
                m_isGenerating = false;
                return;
            }

            // Generate deformations
            GenAPI::GenerationResponse response;
            if (m_animationLength == 1) {
                response = m_generator->generatePose(controlPoints, std::string(m_promptBuffer));
            } else {
                response = m_generator->generateAnimation(controlPoints, std::string(m_promptBuffer), m_animationLength);
            }

            // print response
            std::cout << "Generation successful: " << response.success << std::endl;
            std::cout << "Animation frames generated: " << response.animation_frames.size() << std::endl;

            if (response.success && !response.animation_frames.empty()) {
                // Note: We're in a background thread here, but storeAnimationInMesh only
                // modifies vertex timeframePos maps which should be safe
                bool stored = m_generator->storeAnimationInMesh(m_meshData, response.animation_frames);

                if (stored) {
                    std::cout << "Successfully stored " << response.animation_frames.size() << " animation frames!" << std::endl;
                    std::cout << "Use the timeframe slider to view the animation (1.0, 2.0, 3.0, etc.)" << std::endl;
                } else {
                    m_lastError = "Failed to store animation frames";
                }
            } else {
                m_lastError = response.error_message.empty() ? "Generation failed" : response.error_message;
            }

        } catch (const std::exception& e) {
            m_lastError = "Exception during generation: " + std::string(e.what());
        }

        m_isGenerating = false;
    }).detach();
}

const bool Interface::isHovered() {
    return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}
