#!/bin/bash
set -e

# Create and enter build directory
mkdir -p build
cd build

# Configure with Emscripten toolchain
emcmake cmake ..

# Build the project
cmake --build .

echo "Build complete. Output files:"
