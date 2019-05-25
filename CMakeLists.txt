cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules")

project(nbody LANGUAGES CXX CUDA)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenMP REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)

add_library(nbodyCUDA
  src/nbody.cu
)

set_target_properties(nbodyCUDA PROPERTIES CUDA_SEPARABLE_COMPILATION ON)


add_executable(nbody
  src/nbody.cpp
  src/fb_t.cpp
  src/vao_t.cpp
  include/octree.hpp
  include/fb_t.hpp
  include/nbody.h
)

target_compile_features(nbody PUBLIC cxx_std_11)
target_compile_options(nbody PRIVATE -Wall)

set_target_properties(nbody PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_include_directories(nbody PRIVATE ./include)
target_include_directories(nbody PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(nbody PUBLIC ${GLM_INCLUDE_DIR})
target_link_libraries(nbody PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(nbody PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(nbody PUBLIC GLEW::GLEW)
target_link_libraries(nbody PUBLIC GLUT::GLUT)
target_link_libraries(nbody PUBLIC nbodyCUDA)

if(UNIX)
target_compile_options(nbody PRIVATE -march=native)
endif(UNIX)

if(WIN32)
target_compile_options(nbody PRIVATE /arch:AVX2)
endif(WIN32)