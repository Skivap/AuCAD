#include "Interface.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Interface::Interface(GLFWwindow* window, int screen_width, int screen_height)
    : m_window(window), m_width(screen_width), m_height(screen_height), m_computeDeformedPos(false), safeTimeframe(false), m_weightThreshold(0.1f),
      doRefresh(false), timestep(0.0f)
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

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

const bool Interface::isHovered() {
    return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}
