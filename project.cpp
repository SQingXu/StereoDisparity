#include "redepth.h"

char filename1[] = "/playpen/StereRecording/2/CalibResult3/calib_cam1.txt";
char filename2[] = "/playpen/StereRecording/2/CalibResult3/calib_cam2.txt";

char img1[] = "/playpen/StereRecording/2/output/PointGrey1/00211.jpg";
char img2[] = "/playpen/StereRecording/2/output/PointGrey2/00211.jpg";
int main(void)
{

////    Point2f points(400.5, 200.2);
//    float data1[2] = {400.5, 200.2};
//    Mat points(2,1,CV_32FC1,&data1);
//    Mat unpoints(3,1,CV_32FC1);
//    c2.unprojectPt(points,unpoints,50.0);
////    c1.undistortPt(points,unpoints);
//    printf("result: %10f %10f %10f\n",unpoints.at<float>(0,0), unpoints.at<float>(1,0), unpoints.at<float>(2,0));
//    c1.projectPt(unpoints,points);
//    printf("result: %10f %10f\n",points.at<float>(0,0), points.at<float>(1,0));
    printf("Test\n");
    ReverseDepth rd = ReverseDepth();
    rd.SetRange(50,120);
    rd.ReadImagePair(img1, img2, filename1, filename2);
    Mat depth = rd.FindDepth(false, false, ZNCC);
    //imwrite("ncc_output.jpg",depth);
    namedWindow("Test Window", WINDOW_AUTOSIZE );
    imshow("Test Window",depth);
    waitKey(0);
    return 0;
}
