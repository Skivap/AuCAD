cmake_minimum_required(VERSION 3.11)

# ==========================================
# Configurable Variables
set(PROJECT_NAME_VAR template)
# ==========================================

# Declare Project
project(${PROJECT_NAME_VAR} VERSION 1.0.0 LANGUAGES C CXX)

# C++ Version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# External Libraries (Submodules)
# ==========================================

# Eigen (The CMake of Eigen is shit man)
add_subdirectory(external/eigen)
# GLFW
add_subdirectory(external/glfw)

# ==========================================
# Add Executable
# ==========================================

add_executable(${PROJECT_NAME_VAR}
    src/main.cpp
    src/glad.c
)

# ==========================================
# Link Libraries
# ==========================================

target_link_libraries(${PROJECT_NAME_VAR}
    glfw
)

target_include_directories(${PROJECT_NAME_VAR}
    PRIVATE
        external/eigen
        external/glfw/include
        external/glad/include
)
