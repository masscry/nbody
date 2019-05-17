cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules")

project(nbody CXX)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenMP REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)

add_executable(nbody
  src/nbody.cpp
  include/octree.hpp
)

target_include_directories(nbody PRIVATE ./include)
target_include_directories(nbody PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(nbody PUBLIC ${GLM_INCLUDE_DIR})
target_link_libraries(nbody PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(nbody PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(nbody PUBLIC GLEW::GLEW)
target_link_libraries(nbody PUBLIC GLUT::GLUT)

if(UNIX)
target_compile_options(nbody PRIVATE -march=native)
endif(UNIX)

if(WIN32)
target_compile_options(nbody PRIVATE /arch:AVX2)
endif(WIN32)