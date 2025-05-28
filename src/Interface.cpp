#include "Interface.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Interface::Interface(GLFWwindow* window, int screen_width, int screen_height)
    : m_window(window), m_width(screen_width), m_height(screen_height), m_computeDeformedPos(false), weightUpdate(false), m_weightThreshold(0.1f)
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
    m_visualizeMode = 0; // TODO: Find a way for better visualization
    m_computeDeformedPos = false;
    weightUpdate = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(250, m_height));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Text("Selection Mode:");
    if (ImGui::RadioButton("None", m_selectionMode == SelectionMode::None)) {
        m_selectionMode = SelectionMode::Triangle;
    }
    if (ImGui::RadioButton("Triangle", m_selectionMode == SelectionMode::Triangle)) {
        m_selectionMode = SelectionMode::Triangle;
    }
    if (ImGui::RadioButton("Vertex", m_selectionMode == SelectionMode::Vertex)) {
        m_selectionMode = SelectionMode::Vertex;
    }
    if (ImGui::RadioButton("Axis", m_selectionMode == SelectionMode::AxisDebug)) {
        m_selectionMode = SelectionMode::AxisDebug;
    }

    if (ImGui::Button("Visualize Default")) {
        m_visualizeMode = 1;
    }

    if (ImGui::Button("Visualize Normal")) {
        m_visualizeMode = 2;
    }

    if (ImGui::Button("Visualize Weight")) {
        m_visualizeMode = 3;
    }

    if(ImGui::Button("Compute ARAP")) {
        m_computeDeformedPos = true;
    }

    ImGui::Separator();
    ImGui::Text("Parameters:");
    if(ImGui::SliderFloat("Weight Threshold", &m_weightThreshold, 0.0f, 0.2f, "%.3f", ImGuiSliderFlags_None)) {
        weightUpdate = true;
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

const bool Interface::isHovered() {
    return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}
