project(Lab04)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE glm tinyobjloader)