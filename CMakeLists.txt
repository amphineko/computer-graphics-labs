cmake_minimum_required(VERSION 3.20)
project(cs7gv5_lab1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS})

include_directories(include)

set(shared_files camera.cpp camera.h shader.cpp shader.h program.cpp)
set(shared_libraries ${GLEW_LIBRARIES} glfw ${OpenGL_LIBRARIES})

if (APPLE)
    set(shared_libraries ${shared_libraries} "-framework OpenGL")
endif ()

add_executable(cs7gv5_lab1 cs7gv5_lab1.cpp ${shared_files})
target_link_libraries(cs7gv5_lab1 ${shared_libraries})

include(CMakePrintHelpers)
cmake_print_variables(GLEW_INCLUDE_DIRS)
cmake_print_variables(GLEW_LIBRARIES)
cmake_print_variables(GLFW_INCLUDE_DIRS)
cmake_print_variables(GLFW_LIBRARIES)
