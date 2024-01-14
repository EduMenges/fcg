cmake_minimum_required(VERSION 3.22)
project(FCG)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

if (WIN32)
    if (MINGW)
        set(LIBS ${PROJECT_SOURCE_DIR}/lib/libglfw3.a)
    elseif (MSVC)
        set(LIBS ${PROJECT_SOURCE_DIR}/lib-vc2022/glfw3.lib)
    else ()
        message(FATAL_ERROR "This project file only supports MINGW or MSVC toolchain on Windows.")
    endif ()
    list(APPEND LIBS gdi32 opengl32)
else ()
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    set(LIBS ${PROJECT_SOURCE_DIR}/lib-linux/libglfw3.a X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL)
endif ()

link_libraries(${LIBS})

find_package(glm CONFIG)
if (NOT glm_FOUND)
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 0.9.9.8
    )
    FetchContent_MakeAvailable(glm)
endif ()

find_package(tinyobjloader CONFIG)
if (NOT tinyobjloader_FOUND)
    FetchContent_Declare(
            tinyobjloader
            GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
            GIT_TAG origin/release
    )
    FetchContent_MakeAvailable(tinyobjloader)
endif ()

add_subdirectory(glad)
link_libraries(glad)

add_subdirectory(text)
link_libraries(text)

add_subdirectory(Lab01)
add_subdirectory(Lab02)
add_subdirectory(Lab03)
add_subdirectory(Lab04)
add_subdirectory(Lab05)