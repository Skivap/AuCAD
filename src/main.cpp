#define STB_IMAGE_IMPLEMENTATION

#include "Engine.hpp"

int main(int argc, char** argv) {
#ifdef _OPENMP
    std::cout << "OpenMP is available! Version: " << _OPENMP << std::endl;
#else
    std::cout << "OpenMP is NOT available!" << std::endl;
#endif

    Engine engine;
    engine.run();
    return 0;
}
