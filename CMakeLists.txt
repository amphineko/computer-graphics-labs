cmake_minimum_required(VERSION 3.20)
project(cs7gv5_lab1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLEW 2 REQUIRED)
find_package(glfw3 3 REQUIRED)
find_package(assimp 5 REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS})

include_directories(vendors)
include_directories(vendors/glad/include)
include_directories(vendors/imgui)

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

set(project_shared_files
        vendors/glad/src/glad.c
        vendors/imgui/imgui.cpp
        vendors/imgui/imgui_draw.cpp
        vendors/imgui/imgui_tables.cpp
        vendors/imgui/imgui_widgets.cpp
        vendors/imgui/backends/imgui_impl_glfw.cpp
        vendors/imgui/backends/imgui_impl_opengl3.cpp
        ${project_shared_sources})

set(project_shared_libraries
        ${ASSIMP_LIBRARIES}
        ${GLEW_LIBRARIES}
        glfw
        OpenGL::GL)

if (APPLE)
    set(shared_libraries ${shared_libraries} "-framework OpenGL")
endif ()

add_executable(transmittance transmittance.cpp ${project_shared_files})
target_link_libraries(transmittance ${project_shared_libraries})

add_executable(normalmap normalmap.cpp ${project_shared_files})
target_link_libraries(normalmap ${project_shared_libraries})

include(CMakePrintHelpers)
cmake_print_variables(ASSIMP_INCLUDE_DIRS)
cmake_print_variables(ASSIMP_LIBRARIES)
cmake_print_variables(GLEW_INCLUDE_DIRS)
cmake_print_variables(GLEW_LIBRARIES)
cmake_print_variables(GLFW_INCLUDE_DIRS)
cmake_print_variables(GLFW_LIBRARIES)
cmake_print_variables(OPENGL_INCLUDE_DIRS)
cmake_print_variables(OPENGL_LIBRARIES)
