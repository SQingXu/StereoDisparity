#include "redepth.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WIN_NAME "Depth Output"

char filename1[] = "/playpen/StereRecording/2/CalibResult3/calib_cam1.txt";
char filename2[] = "/playpen/StereRecording/2/CalibResult3/calib_cam2.txt";

char img1[] = "/playpen/StereRecording/2/output/PointGrey1/00211.jpg";
char img2[] = "/playpen/StereRecording/2/output/PointGrey2/00211.jpg";

bool subpixel = true;
bool occlusion = true;
bool regularize = true;
bool savePly = true;
bool colorPly = true;
const char* ply_name = "/playpen/StereoDisparity/Capture/PLY/output_reg.ply";
CostType ct = ZNCC;

const int patch_size = 11;
const int max_iteration = 64;
const float beta = 0.1;
const int reg_iteration = 15;

const float range_min = 50;
const float range_max = 100;
String rtBooleanString(bool bl);

int main(void)
{
    //Set up stereo class variables and image data
    StereoDepthProjection sdp = StereoDepthProjection();
    sdp.ReadImagePair(img1, img2, filename1, filename2);
    sdp.SetStereoVariables(range_min, range_max, patch_size, max_iteration);

    //Find depth with normalized data
    Mat depth = sdp.FindDepth(subpixel,occlusion,ct,regularize,reg_iteration,
                              beta,savePly,colorPly,ply_name);
    Mat normal = sdp.NormalizeRaw(depth);

    namedWindow(WIN_NAME, WINDOW_AUTOSIZE);
    imshow(WIN_NAME,normal);
    imwrite("out_zncc_reg_sub.jpg",normal);

    std::cout << "Depth Information: " << std::endl
              << "  Patch Size: " << patch_size << std::endl
              << "  Max Iteration: " << max_iteration << std::endl
              << "  Range: " << range_min << " - " << range_max << std::endl
              << "  Sub-Pixel: " << rtBooleanString(subpixel) << std::endl
              << "  Occlusion Test: " << rtBooleanString(subpixel) << std::endl
              << "  Regularization: " << rtBooleanString(regularize) << std::endl
              << "  beta: " << beta << std::endl
              << "  Regularization max iteration: " << reg_iteration << std::endl;
    waitKey(0);
    return 0;
}

String rtBooleanString(bool bl){
    if(bl){
        return "true";
    }else{
        return "false";
    }
}
