#pragma once

#ifndef __NBODY_HEADER__
#define __NBODY_HEADER__

int simInitialize(int totalSize, void* ipos, void* ivel);
int simStep(void* inPos, void* outVel, int totalSize);
int simCleanup();

#endif /* __NBODY_HEADER__ */