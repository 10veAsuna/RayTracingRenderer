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

add_library(tinyxml2 STATIC
    third_party/tinyxml2/tinyxml2.cpp
    third_party/tinyxml2/tinyxml2.h
    third_party/tinyxml2/LICENSE.txt
)

add_library(stb_image STATIC
    third_party/stb/stb_image.cpp
    third_party/stb/stb_image.h
    third_party/stb/LICENSE
)
target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb")

add_library(MyRenderCore STATIC
    src/Camera.cpp
    src/Camera.h
    src/Common.h
    src/Disk.cpp
    src/Disk.h
    src/Light.cpp
    src/Light.h
    src/Material.cpp
    src/Material.h
    src/Primitive.cpp
    src/Primitive.h
    src/Ray.cpp
    src/Ray.h
    src/Scene.cpp
    src/Scene.h
    src/SceneObject.cpp
    src/SceneObject.h
    src/SceneXmlLoader.cpp
    src/SceneXmlLoader.h
    src/Sphere.cpp
    src/Sphere.h
    src/Triangle.cpp
    src/Triangle.h
)
target_include_directories(MyRenderCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(MyRenderCore PUBLIC glm stb_image PRIVATE tinyxml2)

add_executable(MyRender
    src/main.cpp
    src/Renderer.cpp
    src/Renderer.h
    scenes/scene08.xml
    scenes/scene09.xml
    scenes/scene_area_light.xml
        src/Texture.h
        src/Texture.cpp
)
target_link_libraries(MyRender PRIVATE MyRenderCore minifb::minifb)

configure_file(scenes/scene08.xml scenes/scene08.xml COPYONLY)
configure_file(scenes/scene09.xml scenes/scene09.xml COPYONLY)
configure_file(scenes/scene_area_light.xml scenes/scene_area_light.xml COPYONLY)
