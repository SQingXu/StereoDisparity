cmake_minimum_required(VERSION 2.8)
project("StereoDisparity")

################################################################################
# Include CMake dependencies
################################################################################

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set (CMAKE_CXX_STANDARD 11)

include(CheckCXXCompilerFlag)

if(EXISTS ${CMAKE_SOURCE_DIR}/LocalConfig.cmake)
    include(${CMAKE_SOURCE_DIR}/LocalConfig.cmake)
endif()


find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)

find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)
find_package(FlyCapture2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

if (OPENMP_FOUND)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(
CUDA_NVCC_FLAGS
${CUDA_NVCC_FLAGS};
-O3 -gencode arch=compute_52,code=sm_52
)



cuda_add_executable(RectifiedDisparity cuda_test.cu)
target_sources(RectifiedDisparity PUBLIC main.cpp cuda_test.h)
target_sources(RectifiedDisparity PUBLIC disparity.cpp disparity.h)

add_executable(ProjectionStereo project.cpp)
target_sources(ProjectionStereo PUBLIC camera.cpp camera.h)
target_sources(ProjectionStereo PUBLIC redepth.cpp redepth.h)

add_executable(FrameExtracter imggenerator.cpp)
target_sources(FrameExtracter PUBLIC dfsv.cpp dfsv.h)

add_executable(FramePlayer mainplayer.cpp)
target_sources(FramePlayer PUBLIC StreamUtil.h)
target_sources(FramePlayer PUBLIC flycaptureplayer.cpp flycaptureplayer.h)
target_sources(FramePlayer PUBLIC DFSVPlayer.cpp DFSVPlayer.h)
target_sources(FramePlayer PUBLIC Calibrator.cpp Calibrator.h)

target_link_libraries(RectifiedDisparity ${OpenCV_LIBS} ${OPENGL_LIBRARIES} GLEW glfw)
target_link_libraries(ProjectionStereo ${OpenCV_LIBS} ${OPENGL_LIBRARIES} GLEW glfw)
target_link_libraries(FrameExtracter ${OpenCV_LIBS} ${OPENGL_LIBRARIES} GLEW glfw)
target_link_libraries(FramePlayer ${OpenCV_LIBS} ${OPENGL_LIBRARIES} GLEW glfw ${FlyCapture2_LIBRARY})

