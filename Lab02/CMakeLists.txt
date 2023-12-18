project(lab_02)

add_executable(lab_02 ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(lab_02 PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(lab_02 PRIVATE glm::glm)
