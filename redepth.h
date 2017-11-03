#ifndef REDEPTH_H
#define REDEPTH_H

#endif // REDEPTH_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "camera.h"

using namespace cv;
using namespace std;

enum CostType{SSD, NCC, ZNCC};
enum Direction{OneToTwo, TwoToOne};

class ReverseDepth{
public:
    const static int patch_size = 7;
    const static int max_iteration = 64;

    bool ReadImagePair(char* file1, char* file2, char* calib1, char* calib2);
    Mat FindDepth(bool subpixel, bool occlusion, CostType ct);
    void SetRange(float min, float max);
private:
    Mat img_a;
    Mat img_b;
    Camera c1;
    Camera c2;
    float range_max;
    float range_min;

    Mat FindDepthRaw(bool subpixel, CostType ct, Direction direct);
    float SSDOfPatches(Mat patch1, Mat patch2);
    float OneSubNCCOfPatches(Mat patch1, Mat patch2);
    float OneSubZNCCOfPatches(Mat patch1, Mat patch2);
    void FuncSelector(CostType ct, float& cost, Mat patch1, Mat patch2);
    Mat NormalizeRaw(Mat raw);
    void GetBilinearPatch(float x, float y, int w_l, int w_r, int h_u, int h_d, Mat img, Mat& patch);
};