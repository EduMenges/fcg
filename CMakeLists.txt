cmake_minimum_required(VERSION 3.22)
project(FCG)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
link_libraries(glfw3)

add_subdirectory(common)
link_libraries(common)
link_libraries(glad)

add_subdirectory(Lab01)
add_subdirectory(Lab02)

