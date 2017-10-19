cmake_minimum_required(VERSION 2.8)
project("StereoDisparity")

find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)

set(
CUDA_NVCC_FLAGS
${CUDA_NVCC_FLAGS};
-O3 -gencode arch=compute_52,code=sm_52
)


cuda_add_executable(RectifiedDisparity cuda_test.cu)
target_sources(RectifiedDisparity PUBLIC main.cpp cuda_test.h)
target_sources(RectifiedDisparity PUBLIC disparity.cpp disparity.h)

target_link_libraries(RectifiedDisparity ${OpenCV_LIBS})
