#include "ProjectionDepth.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ctime>
#include <chrono>
#include <unistd.h>

#define WIN_NAME "Depth Output"
#define PLAY_WIN "Record Video"

const char* calibfile1 = "/playpen/StereoDisparity/Capture/calibrationResult1.txt";
const char* calibfile2 = "/playpen/StereoDisparity/Capture/calibrationResult2.txt";
const char* dfsvfile =  "/playpen/StereoDisparity/Capture/record_2-18";

//char filename1[] = "/playpen/StereRecording/2/CalibResult3/calib_cam1.txt";
//char filename2[] = "/playpen/StereRecording/2/CalibResult3/calib_cam2.txt";
//char img1[] = "/playpen/StereRecording/2/output/PointGrey1/00211.jpg";
//char img2[] = "/playpen/StereRecording/2/output/PointGrey2/00211.jpg";
//char filename1[] = "/playpen/baby/calibrationResult1.txt";
//char filename2[] = "/playpen/baby/calibrationResult2.txt";
//char img1[] = "/playpen/baby/view1.png";
//char img2[] = "/playpen/baby/view5.png";


bool subpixel = true;
bool occlusion = true;
bool regularize = false;
bool savePly = true;
bool colorPly = true;
const char* ply_name = "/playpen/StereoDisparity/Capture/PLY/output_noreg1.ply";
CostType ct = ZNCC;

const int patch_size = 15;
const int max_iteration = 240;
const float beta = 0.1;
const int reg_iteration = 30;

const float range_min = 15;
const float range_max = 40;

String rtBooleanString(bool bl);
void ShowPairImages(vector<StreamPacket>& sp, const String win_name, size_t cv_fmt);
void ReconstructFrame(StereoDepthProjection& sdp, unsigned int frame_num);
void PrintFrameNum(int frame_num);


int main(void)
{
    /* Reconstruction Setup */
    //Set up stereo class variables and image data
    StereoDepthProjection sdp;
    //sdp.ReadImagePair(img1, img2, filename1, filename2);
    sdp.LoadCalibrations(calibfile1,calibfile2);
    sdp.LoadImageDFSV(dfsvfile);


    /* Record Player Setup */
    char keypress;
    DFSVPlayer dfsv_player;
    dfsv_player.Open(dfsvfile);
    std::vector<StreamPacket> packets;
    packets.resize(2);

    unsigned int current_frame = 0;
    unsigned int frame_number = dfsv_player.GetFrameNumber();
    float frame_rate = 30.0;
    bool pause = true;

    cv::namedWindow(PLAY_WIN, WINDOW_AUTOSIZE);
    dfsv_player.SetCurrentFrameNumber(current_frame);
    dfsv_player.GrabNextFrame(packets);
    ShowPairImages(packets, PLAY_WIN, dfsv_player.GetStreamInfo().cvfmt);

    //loop for play the video
    do{
        if(!pause){
            //get time stamp at start of the loop
            chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

            //update current frame number
            current_frame = dfsv_player.GetCurrentFrameNumber();
            current_frame = (current_frame+1)%frame_number;
            dfsv_player.SetCurrentFrameNumber(current_frame);

            dfsv_player.GrabNextFrame(packets);
            ShowPairImages(packets, PLAY_WIN, dfsv_player.GetStreamInfo().cvfmt);

            //get time stamp at end of the loop
            chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

            //sleep the program to match the frame rate
            chrono::duration<double> span = t2 - t1;
            size_t ms = (size_t)(span.count() * 1e6);
            size_t target_span = (size_t)((float)1e6/frame_rate);
            if(target_span > ms){
                usleep(target_span - ms);
            }
        }
        keypress = (cv::waitKey(1) & 0xff);
        if(keypress == 'p'){
            //Pause & Un-pause
            if(pause){
                pause = false;
            }else{
                pause = true;
                PrintFrameNum(current_frame);
            }
        }
        if(keypress == 'r'){
            std::cout << "Start Reconstruct Frame " << current_frame << std::endl;
            PrintFrameNum(current_frame);
            ReconstructFrame(sdp,current_frame);
        }

        if(keypress == 'o'){
            //Move Forward in frame
            if(pause){
                current_frame = (current_frame+1)%frame_number;
                dfsv_player.SetCurrentFrameNumber(current_frame);
                dfsv_player.GrabNextFrame(packets);
                ShowPairImages(packets, PLAY_WIN, dfsv_player.GetStreamInfo().cvfmt);
                PrintFrameNum(current_frame);
            }
        }

        if(keypress == 'i'){
            //Move Backward in frame
            if(pause){
                current_frame = (current_frame-1)%frame_number;
                dfsv_player.SetCurrentFrameNumber(current_frame);
                dfsv_player.GrabNextFrame(packets);
                ShowPairImages(packets, PLAY_WIN, dfsv_player.GetStreamInfo().cvfmt);
                PrintFrameNum(current_frame);
            }
        }
    }while(keypress != 'q');


    return 0;
}

String rtBooleanString(bool bl){
    if(bl){
        return "true";
    }else{
        return "false";
    }
}

void ReconstructFrame(StereoDepthProjection& sdp, unsigned int frame_num){
    sdp.GetFramePairDFSV(frame_num);
    sdp.SetStereoVariables(range_min, range_max, patch_size, max_iteration);
    //Find depth with normalized data
    Mat depth = sdp.FindDepth(subpixel,occlusion,ct,regularize,reg_iteration,
                              beta,savePly,colorPly,ply_name);
    FileStorage fs("raw_disparity.yml", FileStorage::WRITE);
    fs << "raw_mat" << depth;

    //imwrite("raw_disparity.jpg",depth);
    imwrite("color_img.jpg",sdp.getImgColor());
    Mat normal = sdp.NormalizeRaw<uchar>(depth);

    namedWindow(WIN_NAME, WINDOW_AUTOSIZE);
    imshow(WIN_NAME,normal);
    imwrite("depth_stand1.jpg",normal);

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
}

void ShowPairImages(vector<StreamPacket>& sp, const String win_name, size_t cv_fmt){
    std::array<cv::Mat,2> raw_images = {
        sp[0].image_buffer,
        sp[1].image_buffer
    };
    // concatenate for display purposes
    cv::Mat image_pair(raw_images[0].rows, 2 * raw_images[0].cols, cv_fmt);
    cv::hconcat(raw_images[1], raw_images[0], image_pair);
    //std::cout << raw_images[0].rows << " " << raw_images[1].cols << std::endl;

    cv::imshow(win_name, image_pair);
}

void PrintFrameNum(int frame_num){
    std::cout << "Current Frame Number is " << frame_num << std::endl;
}

