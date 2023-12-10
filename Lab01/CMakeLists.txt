project(fan)
project(strip)
project(binary)

add_executable(fan ${PROJECT_SOURCE_DIR}/src/main-fan.cpp)
add_executable(strip ${PROJECT_SOURCE_DIR}/src/main-strip.cpp)
add_executable(binary ${PROJECT_SOURCE_DIR}/src/main-binary.cpp)
