#ifndef DISPARITY_H
#define DISPARITY_H

#endif // DISPARITY_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <math.h>

using namespace cv;
using namespace std;

enum CostType{SSD, NCC, ZNCC};
enum Direction{Left, Right};

class Disparity{
public:
    const static int patch_size = 7;
    const static int max_range = 64;

    bool ReadImagePair(char* file1, char* file2);
    Mat FindDisparity(bool subpixel, bool occlusion, CostType ct);

private:
    Mat img_left;
    Mat img_right;

    Mat FindDisparityRaw(bool subpixel, CostType ct, Direction direct);
    float SSDOfPatches(Mat patch1, Mat patch2);
    float OneSubNCCOfPatches(Mat patch1, Mat patch2);
    float OneSubZNCCOfPatches(Mat patch1, Mat patch2);
    void FuncSelector(CostType ct, float& cost, Mat patch1, Mat patch2);
    Mat NormalizeRaw(Mat raw);

};
