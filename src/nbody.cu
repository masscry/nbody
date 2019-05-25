#include <stdio.h>

inline __host__ __device__ float dot(float3 a, float3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline __host__ __device__ float length(float3 v)
{
    return sqrtf(dot(v, v));
}

const float G = 6.6742867e-5f;

__global__ void simGlobalStep(float3* pos, float3* vel, int total) {
  int idx = threadIdx.x + blockIdx.x*blockDim.x;

  if (idx < total) {
    float3 force =  make_float3(0.0f, 0.0f, 0.0f);

    for (int j = 0; j < total; ++j) {
      float3 dlt;
      float sqlen;
      float len;

      if (j == idx) {
        continue;
      }

      dlt.x = pos[j].x - pos[idx].x;
      dlt.y = pos[j].y - pos[idx].y;
      dlt.z = pos[j].z - pos[idx].z;

      sqlen = dot(dlt, dlt);

      len = sqrtf(sqlen);

      dlt.x /= len;
      dlt.y /= len;
      dlt.z /= len;

      sqlen = (sqlen < 1.0f) ? 1.0f : sqlen;

      force.x += dlt.x * G / sqlen;
      force.y += dlt.y * G / sqlen;
      force.z += dlt.z * G / sqlen;
    }

    vel[idx].x += force.x;
    vel[idx].y += force.y;
    vel[idx].z += force.z;
  }
}

void* cudaPosData;
void* cudaVelData;

int simInitialize(int totalSize, void* ipos, void* ivel) {

  cudaError_t error;

  error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s:%d: CUDA: %s (%d)\n", __FILE__, __LINE__, cudaGetErrorString(error), error);
    return -1;
  }

  cudaMalloc(&cudaPosData, totalSize*sizeof(float3));
  cudaDeviceSynchronize();
  error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s:%d: CUDA: %s (%d)\n", __FILE__, __LINE__, cudaGetErrorString(error), error);
    return -1;
  }
  cudaMemcpy(cudaPosData, ipos, totalSize*sizeof(float3), cudaMemcpyHostToDevice);

  cudaMalloc(&cudaVelData, totalSize*sizeof(float3));
  cudaDeviceSynchronize();
  error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s:%d: CUDA: %s (%d)\n", __FILE__, __LINE__, cudaGetErrorString(error), error);
    return -1;
  }
  cudaMemcpy(cudaVelData, ivel, totalSize*sizeof(float3), cudaMemcpyHostToDevice);
  return 0;
}

int simStep(void* inPos, void* outVel, int totalSize) {
  cudaError_t error;

  cudaMemcpy(cudaPosData, inPos, totalSize*sizeof(float3), cudaMemcpyHostToDevice);
  simGlobalStep<<<totalSize/512,512>>>((float3*)cudaPosData, (float3*)cudaVelData, totalSize);
  cudaMemcpy(outVel, cudaVelData, totalSize*sizeof(float3), cudaMemcpyDeviceToHost);
  cudaDeviceSynchronize();

  error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s:%d: CUDA: %s (%d)\n", __FILE__, __LINE__, cudaGetErrorString(error), error);
    return -1;
  }

  return 0;
}

int simCleanup() {
  cudaFree(cudaPosData);
  cudaFree(cudaVelData);
  return 0;
}