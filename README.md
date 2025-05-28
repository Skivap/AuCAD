## Using Project

#### 1. Pull the repository
Clone this repository
`git clone --recurse-submodules https://github.com/Skivap/OpenGL-Template.git`

#### 2. Create ./build folder in your repository
`mkdir build`
Or manually create it

#### 3. Build the project using CMake
1. `cd build`
2. `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..` cmake configuration
3. `cmake --build .` cmake build project
4. `./{your_project_name}` to execute

Or you can just go to the project directory and run:
`bash compile.sh`

#### 3.5 Compiling Issue: libigl & Eigen Compability

In voronoi_mass.cpp inside libigl, change line 50
```cpp
I = Ivec.reshaped(T.rows(),T.cols());
```
to
```cpp
Eigen::Map<const Eigen::MatrixXi>(Ivec.data(), T.rows(), T.cols());
```

Also, inside the `placeholder.h` change the Eigen version from 3.4 to 3.3
```cpp
#if EIGEN_VERSION_AT_LEAST(3, 3, 90)
  #define IGL_PLACEHOLDERS_ALL Eigen::placeholders::all
#else
  #define IGL_PLACEHOLDERS_ALL Eigen::all
#endif
```

Our Eigen Version need the namespace of Eigen::placeholders::all, but it was not detected


#### 4. Run the executable
After compilation, you can run the executable directly from the build directory `./build/{your_project_name}`
