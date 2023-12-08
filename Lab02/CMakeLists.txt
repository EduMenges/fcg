project(lab_02)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(lab_02 ${PROJECT_SOURCE_DIR}/src/main.cpp ${PROJECT_SOURCE_DIR}/src/textrendering.cpp)

target_include_directories(lab_02 PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(lab_02 PRIVATE glad::glad)
target_link_libraries(lab_02 PRIVATE glfw)
target_link_libraries(lab_02 PRIVATE glm::glm)
