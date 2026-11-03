cmake_minimum_required(VERSION 3.16)
project(MyRender LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLM is a header-only library.
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm")

# Build MiniFB from the source included with this project.
set(MINIFB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MINIFB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MINIFB_BUILD_VERSION_INFO OFF CACHE BOOL "" FORCE)
if(WIN32)
    set(MINIFB_USE_OPENGL_API OFF CACHE BOOL "Use MiniFB's GDI backend" FORCE)
endif()
add_subdirectory(third_party/minifb)

file(GLOB_RECURSE MYRENDER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)
add_executable(MyRender ${MYRENDER_SOURCES}
        src/Camera.h
        src/Sphere.h
        src/Disk.h
        src/Triangle.h)
target_link_libraries(MyRender PRIVATE glm minifb::minifb)
