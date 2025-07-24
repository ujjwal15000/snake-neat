cmake_minimum_required(VERSION 3.16)
project(SnakeGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# ---------- Force Homebrew LLVM (for OpenMP support) ----------
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

# Add LLVM and libomp include/link flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -I/opt/homebrew/opt/llvm/include")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/opt/libomp/lib -lomp -L/opt/homebrew/opt/llvm/lib")

# -------------------- SDL2 --------------------
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# -------------------- SnakeGame Library --------------------
add_library(snakegame
        src/Snake.cpp
        src/Game.cpp
)
target_include_directories(snakegame PUBLIC include)

# -------------------- Executable --------------------
add_executable(snakeapp
        src/main.cpp
        src/Renderer.cpp
        src/SDLInputProvider.cpp
        src/ModelInputProvider.cpp
        src/Model.cpp
        src/Population.cpp
)
target_include_directories(snakeapp PRIVATE include)

target_link_libraries(snakeapp
        PRIVATE
        snakegame
        ${SDL2_LIBRARIES}
)
