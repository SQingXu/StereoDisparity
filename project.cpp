#include "camera.h"

char filename2[] = "/playpen/StereRecording/1/CalibResult4/calib_cam2.txt";
char filename1[] = "/playpen/StereRecording/1/CalibResult4/calib_cam1.txt";
int main(void)
{
    Camera c1 = Camera();
    Camera c2 = Camera();
    c1.readCalibration(filename1);
    c2.readCalibration(filename2);
//    Point2f points(400.5, 200.2);
    float data1[2] = {400.5, 200.2};
    Mat points(2,1,CV_32FC1,&data1);
    Mat unpoints(3,1,CV_32FC1);
    c2.unprojectPt(points,unpoints,50.0);
//    c1.undistortPt(points,unpoints);
    printf("result: %10f %10f %10f\n",unpoints.at<float>(0,0), unpoints.at<float>(1,0), unpoints.at<float>(2,0));
    c1.projectPt(unpoints,points);
    printf("result: %10f %10f\n",points.at<float>(0,0), points.at<float>(1,0));
    return 0;
}
