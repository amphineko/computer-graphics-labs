cmake_minimum_required(VERSION 3.20)
project(cs7gv5_lab1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLEW 2 REQUIRED)
find_package(glfw3 3 REQUIRED)
find_package(assimp 5 REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS})

include_directories(vendors)

add_definitions(-DSTB_IMAGE_IMPLEMENTATION)

set(project_shared_sources
        lib/cameras/camera_fp.h
        lib/cameras/camera_tp.h
        lib/cameras/common.h
        lib/models/mesh.h
        lib/models/node.h
        lib/models/scene.h
        lib/models/textures.h
        lib/shaders/shader.h
        lib/program.h)

set(project_shared_files ${project_shared_sources} lib/stdafx.h)

set(project_shared_libraries
        ${ASSIMP_LIBRARIES}
        ${GLEW_LIBRARIES}
        glfw
        OpenGL::GL)

if (APPLE)
    set(shared_libraries ${shared_libraries} "-framework OpenGL")
endif ()

add_executable(transmittance transmittance.cpp ${project_shared_files} transmittance.cpp)
target_link_libraries(transmittance ${project_shared_libraries})

include(CMakePrintHelpers)
cmake_print_variables(ASSIMP_INCLUDE_DIRS)
cmake_print_variables(ASSIMP_LIBRARIES)
cmake_print_variables(GLEW_INCLUDE_DIRS)
cmake_print_variables(GLEW_LIBRARIES)
cmake_print_variables(GLFW_INCLUDE_DIRS)
cmake_print_variables(GLFW_LIBRARIES)
cmake_print_variables(OPENGL_INCLUDE_DIRS)
cmake_print_variables(OPENGL_LIBRARIES)
