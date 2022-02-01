cmake_minimum_required(VERSION 3.20)
project(cs7gv5_lab1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(assimp 5.2 REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS})

include_directories(vendors)

add_definitions(-DSTB_IMAGE_IMPLEMENTATION)

set(project_shared_sources
        cameras/camera_fp.h
        cameras/camera_tp.h
        cameras/common.h
        model.cpp
        model.h
        program.cpp
        program.h
        shader.cpp
        shader.h
        utah.cpp
        utah.h)

set(project_shared_files ${project_shared_sources})

set(project_shared_libraries
        ${ASSIMP_LIBRARIES}
        ${GLEW_LIBRARIES}
        glfw
        OpenGL::GL)

if (APPLE)
    set(shared_libraries ${shared_libraries} "-framework OpenGL")
endif ()

add_executable(cs7gv5_lab1 cs7gv5_lab1.cpp ${project_shared_files})
target_link_libraries(cs7gv5_lab1 ${project_shared_libraries})

add_executable(utah utah_main.cpp ${project_shared_files})
target_link_libraries(utah ${project_shared_libraries})

include(CMakePrintHelpers)
cmake_print_variables(ASSIMP_INCLUDE_DIRS)
cmake_print_variables(ASSIMP_LIBRARIES)
cmake_print_variables(GLEW_INCLUDE_DIRS)
cmake_print_variables(GLEW_LIBRARIES)
cmake_print_variables(GLFW_INCLUDE_DIRS)
cmake_print_variables(GLFW_LIBRARIES)
cmake_print_variables(OPENGL_INCLUDE_DIRS)
cmake_print_variables(OPENGL_LIBRARIES)
