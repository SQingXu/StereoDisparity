
#include "disparity.h"

using namespace std;
using namespace cv;

char img1[] = "im2.png";
char img2[] = "im6.png";
char data_dir[] = "/playpen/teddy";
char filename1[30];
char filename2[30];
int main()
{
    sprintf(filename1, "%s/%s",data_dir, img1);
    sprintf(filename2, "%s/%s",data_dir, img2);
//    getchar();
//    printf("width: %d; height: %d; step: %d\n", img1.cols,img1.rows,(int)img1.step);
//    for(int i = 0; i < img1.rows; i++){
//        for(int j = 0; j < img1.cols; j++){
//            img1.data[i*img1.step + j] = 0;
//        }
//    }

    Disparity d = Disparity();
    if(!d.ReadImagePair(filename1, filename2)){
        return 1;
    }

    Mat dis = d.FindDisparity(true,true,NCC);
    Mat img1 = imread(filename1, CV_LOAD_IMAGE_GRAYSCALE);
    imwrite("Disparity.jpg",dis);
    namedWindow("Test Window", WINDOW_AUTOSIZE );
    imshow("Test Window",dis);
    waitKey(0);
    return 0;
}
