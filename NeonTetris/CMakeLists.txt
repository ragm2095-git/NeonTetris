cmake_minimum_required(VERSION 3.21)
project(NeonTetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(NeonTetris
    src/main.cpp
    src/Renderer.cpp
    src/TetrisGame.cpp
    src/Tetromino.cpp)

target_include_directories(NeonTetris PRIVATE include)
target_link_libraries(NeonTetris PRIVATE SFML::Graphics SFML::Window SFML::System)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(NeonTetris PRIVATE -Wall -Wextra)
endif()
