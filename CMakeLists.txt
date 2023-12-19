cmake_minimum_required(VERSION 3.22)
project(FCG)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(glfw3 CONFIG REQUIRED)
link_libraries(glfw)

find_package(glm CONFIG)
if (NOT glm_FOUND)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
  )
  FetchContent_MakeAvailable(glm)
endif()

add_subdirectory(common)
link_libraries(common)
link_libraries(glad)

add_subdirectory(Lab01)
add_subdirectory(Lab02)
add_subdirectory(Lab03)