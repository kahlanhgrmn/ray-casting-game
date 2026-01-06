cmake_minimum_required(VERSION 3.16)
project(MazeExplorer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.0
)

FetchContent_MakeAvailable(raylib)

add_executable(MazeExplorer main.cpp)
target_link_libraries(MazeExplorer PRIVATE raylib)

# On some platforms you may need:
# target_link_libraries(MazeExplorer PRIVATE raylib m pthread dl rt X11)
