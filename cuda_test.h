#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cuda_runtime.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>

#define N 512
#define DFS_CUDA_ASSERT(e) { cudaAssert((e), __FILE__, __LINE__); }
inline void cudaAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
if (code != cudaSuccess)
{
fprintf(stderr,"CudaAssert: %s %s %d\n", cudaGetErrorString(code), file, line);
if (abort)
exit(code);
}
}
__global__
void add_v(int *a, int *b, int *c);
void random_ints(int *a, int range);
void print_vector(int* a);
