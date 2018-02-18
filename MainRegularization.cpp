#include "Regularizer.h"
#include "PLYConverter.h"

#define RWINDOW "Regularized"

Mat readDisparity(String img_path);
int main(){
    //pre-set data for regularization
    String path = "/playpen/StereoDisparity/build_debug/raw_disparity.jpg";
    String color_path = "/playpen/StereoDisparity/build_debug/color_img.jpg";
    const char* calibfile1 = "/playpen/StereoDisparity/Capture/calibrationResult1.txt";
    const char* ply_name = "/playpen/StereoDisparity/Capture/PLY/output_reg1.ply";
    const float beta = 0.2;
    const int iteration = 15;
    bool savePly = true;

    Regularizer rg;
    Mat res;
    Mat disparity = readDisparity(path);

    if(!disparity.data){
        cout << "invalid file name" << endl;
        return 0;
    }

    if(disparity.type() != CV_32FC1){
       cout << "Unexpected Image Type" << endl;
       return 0;
    }

    rg.Start(disparity.rows * disparity.cols);
    rg.Regularize(disparity,res,beta, iteration);
    if(savePly){
        PLYConverter converter;
        Mat color = imread(color_path,CV_LOAD_IMAGE_GRAYSCALE);
        Camera c1 = Camera();
        c1.readCalibration(calibfile1);
        converter.writeDepthToPLY(ply_name,res,color,c1,true);
    }
    namedWindow(RWINDOW,WINDOW_AUTOSIZE);
    imshow(RWINDOW, res);
    waitKey(0);
return 0;
}

Mat readDisparity(String img_path){
    Mat disparity = imread(img_path,CV_LOAD_IMAGE_GRAYSCALE);


    return disparity;

}
