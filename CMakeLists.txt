cmake_minimum_required(VERSION 3.11)

# ==========================================
# Configurable Variables
set(PROJECT_NAME_VAR app)
# ==========================================

# Declare Project
project(${PROJECT_NAME_VAR} VERSION 1.0.0 LANGUAGES C CXX)

# C++ Version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# External Libraries (Submodules)
# ==========================================

# Eigen (The CMake of Eigen is shit man)
add_subdirectory(external/eigen)
# GLFW
add_subdirectory(external/glfw)
# Assimp
add_subdirectory(external/assimp)
# libigl
add_subdirectory(external/libigl)

# ==========================================
# Add Executable
# ==========================================

add_executable(${PROJECT_NAME_VAR}
    src/main.cpp
    src/Engine.cpp
    src/Renderer.cpp
    src/Trackball.cpp
    src/Interface.cpp

    # Visualizer
    src/Visualizer/BaseObject.cpp
    src/Visualizer/Wireframe.cpp
    src/Visualizer/Mesh.cpp
    src/Visualizer/PointCloud.cpp
    src/Visualizer/Axis.cpp

    # Mesh Utilities
    src/Mesh/MeshData.cpp
    src/Mesh/MeshSelection.cpp
    src/Mesh/MeshProcessor.cpp
    src/Mesh/MeshBuffer.cpp

    # Gizmo Utilities
    src/Gizmo/Gizmo.cpp

    # imgui
    external/imgui/src/imgui.cpp
    external/imgui/src/imgui_demo.cpp
    external/imgui/src/imgui_draw.cpp
    external/imgui/src/imgui_tables.cpp
    external/imgui/src/imgui_widgets.cpp
    external/imgui/src/imgui_impl_opengl3.cpp
    external/imgui/src/imgui_impl_glfw.cpp

    # glad
    external/glad/src/glad.c
)

# ==========================================
# Link Libraries
# ==========================================

target_link_libraries(${PROJECT_NAME_VAR}
    glfw
    assimp::assimp
)

target_include_directories(${PROJECT_NAME_VAR}
    PRIVATE
        external/eigen
        external/glfw/include
        external/glad/include
        external/imgui/include
        external/stb_image/include
        external/assimp/include
        external/libigl/include
)
