#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cuda_runtime.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>

__global__
void FindDepthRaw(ushort* orig_a, ushort* orig_b);
//void
//void LoadMatToDev();
