project(Lab02)

add_executable(Lab02 ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(Lab02 PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(Lab02 PRIVATE glm)
