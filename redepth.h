#ifndef REDEPTH_H
#define REDEPTH_H

#endif // REDEPTH_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "camera.h"
#include "PLYConverter.h"
#include "Regularizer.h"

using namespace cv;
using namespace std;

enum CostType{SSD, NCC, ZNCC};
enum Direction{OneToTwo, TwoToOne};

class StereoDepthProjection{
public:

    bool ReadImagePair(char* file1, char* file2, char* calib1, char* calib2);
    Mat FindDepth(bool subpixel, bool occlusion, CostType ct,
                  bool regularize, bool iteration = 15, const float beta = 0.1f,
                  bool SavePly = true, bool isColor = true,
                  const char* path = "/playpen/StereoDisparity/Capture/PLY/output_reg.ply");
    void SetStereoVariables(float min, float max, int win_size, int max_iter){
        range_min = min;
        range_max = max;
        patch_size = win_size;
        max_iteration = max_iter;
    }
    Mat NormalizeRaw(Mat raw);
private:
    int patch_size;
    int max_iteration;
    Mat img_color;
    Mat img_a;
    Mat img_b;
    Camera c1;
    Camera c2;
    float range_max;
    float range_min;

    Mat FindDepthRaw(bool subpixel, CostType ct, Direction direct, bool regularize, Regularizer& rg);
    float SSDOfPatches(Mat patch1, Mat patch2);
    float OneSubNCCOfPatches(Mat patch1, Mat patch2);
    float OneSubZNCCOfPatches(Mat patch1, Mat patch2);
    void FuncSelector(CostType ct, float& cost, Mat patch1, Mat patch2);
    bool GetBilinearPatch(float x, float y, int w_l, int w_r, int h_u, int h_d, Mat img, Mat& patch);
};
