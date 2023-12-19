project(lab_03)

add_executable(lab_03 ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(lab_03 PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(lab_03 PRIVATE glm)
