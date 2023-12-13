project(lab_03)

add_executable(lab_03 ${PROJECT_SOURCE_DIR}/src/main.cpp ${PROJECT_SOURCE_DIR}/src/textrendering.cpp)
target_include_directories(lab_03 PRIVATE ${PROJECT_SOURCE_DIR}/include)

find_package(glm CONFIG REQUIRED)
target_link_libraries(lab_03 PRIVATE glm)
