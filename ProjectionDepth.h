#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include "Camera.h"
#include "PLYConverter.h"
#include "Regularizer.h"
#include "DFSVPlayer.h"


using namespace cv;
using namespace std;

enum CostType{SSD, NCC, ZNCC};
enum Direction{OneToTwo, TwoToOne};

class StereoDepthProjection{
public:

    bool ReadImagePair(char* file1, char* file2, char* calib1, char* calib2);
    bool LoadCalibrations(const char* calib1, const char* calib2);
    bool LoadImageDFSV(const char* file);
    bool GetFramePairDFSV(unsigned int frame);
    Mat getImgColor(){return img_color;}
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
    template <typename T>
    inline Mat NormalizeRaw(Mat raw){
        if(!raw.data){
            return Mat(100,100,CV_8UC1);
        }
        int rows = raw.rows;
        int cols = raw.cols;
        Mat rt(rows, cols, CV_8UC1);
        for(int r = 0; r < rows; r++){
            for(int c = 0; c < cols; c++){
                float val = raw.at<float>(r,c);
                if(val <= 0.0){
                    rt.at<T>(r,c) = 0;
                    continue;
                }
                rt.at<T>(r,c) = (int)((val/range_max)* (2 << sizeof(T)*8));
            }
        }
        return rt;
    }

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
    DFSVPlayer dplayer;
    bool player_ini = false;

    void ConvertGray16ToGray8(Mat sixteenBit, Mat &eightBit);
    template <typename T> Mat FindDepthRaw(bool subpixel, CostType ct, Direction direct, bool regularize, Regularizer& rg);
    template <typename T> float SSDOfPatches(Mat patch1, Mat patch2);
    template <typename T> float OneSubNCCOfPatches(Mat patch1, Mat patch2);
    template <typename T> float OneSubZNCCOfPatches(Mat patch1, Mat patch2);
    template <typename T> void FuncSelector(CostType ct, float& cost, Mat patch1, Mat patch2);
    template <typename T> bool GetBilinearPatch(float x, float y, int w_l, int w_r, int h_u, int h_d, Mat img, Mat& patch);
};
