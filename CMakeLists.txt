cmake_minimum_required(VERSION 3.16)
project(SnakeGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Find SDL2 Package
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# SnakeGame Library
add_library(snakegame
        src/Snake.cpp
        src/Game.cpp
)

target_include_directories(snakegame PUBLIC include)

# SnakeApp Executable
add_executable(snakeapp
        src/main.cpp
        src/Renderer.cpp
        src/SDLInputProvider.cpp
        src/Model.cpp
)

target_include_directories(snakeapp PRIVATE include)
target_link_libraries(snakeapp PRIVATE snakegame ${SDL2_LIBRARIES})
