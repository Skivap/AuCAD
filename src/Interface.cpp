#include "Interface.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Mesh/MeshData.hpp"

Interface::Interface(GLFWwindow* window, int screen_width, int screen_height)
    : m_window(window), m_width(screen_width), m_height(screen_height), m_computeDeformedPos(false), safeTimeframe(false), m_weightThreshold(0.1f),
      doRefresh(false), timestep(0.0f), m_meshData(nullptr), m_showVertexPanel(true)
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
    if (ImGui::Button("Safe Timeframe")) safeTimeframe = true;

    if (ImGui::SliderFloat("Timestep", &timestep, 0.0f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp)) {
        doRefresh = true;
    }

    ImGui::Separator();

    ImGui::Text("Panels:");
    ImGui::Checkbox("Show Vertex Panel", &m_showVertexPanel);

    ImGui::End();

    // Draw vertex panel
    drawVertexPanel();

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

const bool Interface::isHovered() {
    return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}
