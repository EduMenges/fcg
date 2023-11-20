cmake_minimum_required(VERSION 3.22)

project(Lab01)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(Lab01 ${SOURCE_FILES})

target_include_directories(Lab01 PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(Lab01 PRIVATE glfw)
target_link_libraries(Lab01 PRIVATE glad::glad)
