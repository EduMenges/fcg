cmake_minimum_required(VERSION 3.22)
project(FCG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_libraries(glfw)
link_libraries(glad::glad)

add_subdirectory(Lab01)
