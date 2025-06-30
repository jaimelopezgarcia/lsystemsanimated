cmake_minimum_required(VERSION 3.10)
project(lsystemsanimated)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Emscripten as the toolchain if not already set
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Please run cmake with Emscripten toolchain: emcmake cmake .")
endif()

# Add include directory
include_directories(include)

# Add source files
file(GLOB SOURCES src/*.cpp)

# Create the executable (WASM + JS glue)
add_executable(lsystemsanimated ${SOURCES})

# Optional: Set output name and properties for Emscripten
set_target_properties(lsystemsanimated PROPERTIES
  OUTPUT_NAME     "lsystemsanimated"
  SUFFIX          ".js"
  LINK_FLAGS      "-s WASM=1 -s EXPORT_NAME='\"Module\"' \
                   -s ALLOW_MEMORY_GROWTH=1 \
                   -s EXPORTED_FUNCTIONS=\"['_lsystem_physics_create','_lsystem_physics_step','_lsystem_physics_destroy','_free_float_buffer']\" \
                   -s EXTRA_EXPORTED_RUNTIME_METHODS=\"['cwrap']\""
)

# Optional: Emscripten linker flags (for example, to enable WebGL, filesystem, etc.)
set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1")

# Optional: Copy assets (if you have any)
# file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})