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

# Add all source files
add_executable(MazeExplorer 
    main.cpp
    map.cpp
    player.cpp
    raycaster.cpp
)

target_link_libraries(MazeExplorer PRIVATE raylib)

add_custom_command(TARGET MazeExplorer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:MazeExplorer>/assets
)

# On some platforms you may need:
# target_link_libraries(MazeExplorer PRIVATE raylib m pthread dl rt X11)