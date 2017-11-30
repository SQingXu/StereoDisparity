#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace cv;
using namespace std;

class Camera{
public:
    int width, height;
    bool readCalibration(char* filename);
    bool projectPt(Mat point_3d, Mat& pixel_cord);
    bool unprojectPt(Mat pixel_cord, Mat& point_3d,float depth);
    bool worldToCamPt(Mat point_w, Mat& point_c);
    bool distortPt(Mat orig, Mat& dist);
    bool undistortPt(Mat dist, Mat& orig);

private:
    Mat K, K_inv; //intrinsics 3x3
    Mat R; //extrinsics rotation matrix 3x3
    Mat t; //translation 3x1
    Mat E, E_inv;
    Mat dist;
    float k1, k2, k3, p1, p2; //distortion coefficient
};
