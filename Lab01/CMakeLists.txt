project(fan)
project(strip)
project(binary)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

link_libraries(glfw)
link_libraries(glad::glad)

add_executable(fan ${PROJECT_SOURCE_DIR}/src/main-fan.cpp)
add_executable(strip ${PROJECT_SOURCE_DIR}/src/main-strip.cpp)
add_executable(binary ${PROJECT_SOURCE_DIR}/src/main-binary.cpp)
