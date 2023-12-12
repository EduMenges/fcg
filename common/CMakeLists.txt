project(common)
add_library(${PROJECT_NAME} textrendering.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

add_subdirectory(glad)