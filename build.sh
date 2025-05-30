set -e

if [[ "$OSTYPE" == "darwin"* ]]; then
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
fi

mkdir -p build
cd build

cmake .. \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build .

cd ..
