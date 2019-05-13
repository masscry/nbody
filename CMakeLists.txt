cmake_minimum_required(VERSION 3.10)

project(nbody CXX)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(nbody
  src/nbody.cpp
)

target_include_directories(nbody PUBLIC ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
target_link_libraries(nbody PUBLIC ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
