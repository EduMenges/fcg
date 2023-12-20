project(Lab03)

add_executable(Lab03 ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(Lab03 PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(Lab03 PRIVATE glm)
