cmake_minimum_required(VERSION 3.16)
project(physics)

# Raylib works well with C++17 or C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where to find Raylib (adjust path if necessary)
find_package(raylib REQUIRED)

add_executable(physics src/main.cpp)

# Link Raylib and required system frameworks on macOS
target_link_libraries(physics raylib m
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
)
