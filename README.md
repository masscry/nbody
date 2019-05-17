# nbody

Barnes-Hut N-body simulation algorithm implementation

## Requirements

 * OpenGL 3.3
 * OpenMP 2.0
 * freeglut
 * glew
 * glm

## Brief introduction

This is implementation of classical Barnes-Hut N-body simulation algorithm.

All particles has mass of 1 kg, G constant increased for faster simulation.

Particles rendering uses point sprite technique. Shaders are used to change 
particle size according to distance from camera.

## Sources

 * nbody.cpp - main entry point, OpenGL rendering related stuff, and overall 
 management.
 * octree.hpp - octree implementation to be used in Barnes-Hut algorithm.
 * resources.inl - shader releated stuff.

## Implementation details

 * Simulation processed with constant time step on each glutIdleFunc invocation.
 After each world update, screen redraw requested. (see nbody.cpp onIdle function)
 * No optimization techniques are used to ease octree building, because time 
 to build it from scratch in negligible in comparison with time spent in 
 ocleaf_t::GetForceOnPoint function - 99% of simulation time program sits in 
 ocleaf_t::GetForceOnPoint.

## Configuration flags

There are some configuration flags in program, which affects simulation and its
presentation:

 * __BRUTE_FORCE__ - when defined, simulation uses brute-force N^2 algorithm
 * __DEBUG_OCTREE__ - when defined, octree leaves with particles inside are
 drawn.
 * __WND_MODE__ - when defined, simulation runs in windowed mode, otherwise
 in fullscreen 1920x1080.
 
