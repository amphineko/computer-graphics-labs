cmake_minimum_required(VERSION 3.20)
project(cs7gv5_lab1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(assimp 5.2 REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS})

include_directories(include)

set(shared_files camera.cpp camera.h shader.cpp shader.h program.cpp utah.cpp utah.h program.h model.cpp model.h)
set(shared_libraries ${ASSIMP_LIBRARIES} ${GLEW_LIBRARIES} glfw OpenGL::GL)

if (APPLE)
    set(shared_libraries ${shared_libraries} "-framework OpenGL")
endif ()

add_executable(cs7gv5_lab1 cs7gv5_lab1.cpp ${shared_files})
target_link_libraries(cs7gv5_lab1 ${shared_libraries})

add_executable(utah utah_main.cpp ${shared_files})
target_link_libraries(utah ${shared_libraries})

include(CMakePrintHelpers)
cmake_print_variables(ASSIMP_INCLUDE_DIRS)
cmake_print_variables(ASSIMP_LIBRARIES)
cmake_print_variables(GLEW_INCLUDE_DIRS)
cmake_print_variables(GLEW_LIBRARIES)
cmake_print_variables(GLFW_INCLUDE_DIRS)
cmake_print_variables(GLFW_LIBRARIES)
cmake_print_variables(OPENGL_INCLUDE_DIRS)
cmake_print_variables(OPENGL_LIBRARIES)
