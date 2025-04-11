cd ./build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
cmake --build .
cd ..
