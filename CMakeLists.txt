cmake_minimum_required(VERSION 3.20)
project(cs7gv5_lab1)

set(CMAKE_CXX_STANDARD 20)


find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

add_executable(cs7gv5_lab1 cs7gv5_lab1.cpp)
target_link_libraries(cs7gv5_lab1 ${OpenGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES})

if (APPLE)
    target_link_libraries(cs7gv5_lab1 "-framework OpenGL")
endif()

#include(CMakePrintHelpers)
#cmake_print_variables(GLEW_INCLUDE_DIRS)
#cmake_print_variables(GLEW_LIBRARIES)
#cmake_print_variables(GLUT_INCLUDE_DIRS)
#cmake_print_variables(GLUT_LIBRARIES)
