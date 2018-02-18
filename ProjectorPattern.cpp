#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <math.h>
#include <vector>

#define WINDOW "Pattern"

using namespace cv;
using namespace std;

Mat randomPattern(int width, int height, int patch, int neighbor);
uchar randomTwoStage(int stage);
int main(){
    int width = 1080;
    int height= 810;
    int patch = 9;
    int neighbor = 10;
    char keypress;
    Mat pattern = randomPattern(width,height,patch,neighbor);
    namedWindow(WINDOW, WINDOW_AUTOSIZE);
    imshow(WINDOW, pattern);
    imwrite("projector_pattern.jpg",pattern);
    waitKey(0);
    return 0;
}

Mat randomPattern(int width, int height,int patch, int neighbor){
    Mat img(height,width, CV_8UC1);
    for(int r = 0; r < height; r+=patch){
        int row_offset = ((int)((int)r/patch)/neighbor)%2;
        for(int c = 0; c < width; c+=patch){
            int bi_state = (((int)((int)c/patch)/neighbor) + row_offset)%2 ;
            int sr_l = min(height-r,patch);
            int sc_l = min(width-c,patch);
//            int pat_val = randomTwoStage(bi_state);
            int pat_val = rand()%255;
            for(int sr = 0; sr < sr_l; sr++){
                for(int sc = 0; sc < sc_l; sc++){
                    img.at<uchar>(r+sr, c+sc) = pat_val;
                }
            }
        }
    }
    cout << "Pattern complete generation" << endl;
    return img;
}

uchar randomTwoStage(int stage){
    int val = rand()%127 + stage * 128;
    return (uchar)val;
}
