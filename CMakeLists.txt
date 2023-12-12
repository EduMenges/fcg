cmake_minimum_required(VERSION 3.22)
project(FCG)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
find_package(glfw3 CONFIG)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
        glfw3
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    FetchContent_MakeAvailable(glfw3)
endif ()

link_libraries(glfw3)

add_subdirectory(common)
link_libraries(common)
link_libraries(glad)

add_subdirectory(Lab01)
add_subdirectory(Lab02)

