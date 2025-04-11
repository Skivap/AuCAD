## Using Project

#### 1. Pull the repository
Clone this repository
`git clone --recurse-submodules https://github.com/Skivap/OpenGL-Template.git`

#### 2. Build the project using CMake
1. `cd build`
2. `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..` cmake configuration
3. `cmake --build .` cmake build project
4. `./{your_project_name}` to execute

Or you can just go to the project directory and run

`bash compile.sh`

#### 3. Run the executable
After compilation, you can run the executable directly from the build directory `./build/{your_project_name}`

## Project Information
#### Glad Specification
- Specification : OpenGL
- gl version    : 4.6
- Profile       : Compability

#### GLFW Specification
Based on the submodule

#### Eigen Specification
Based on the submodule
