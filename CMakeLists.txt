cmake_minimum_required(VERSION 2.8)
project("StereoDisparity")

find_package(OpenCV REQUIRED)
add_executable(RectifiedDisparity main.cpp disparity.cpp disparity.h)

target_link_libraries(RectifiedDisparity ${OpenCV_LIBS})
