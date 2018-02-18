#include "RectifiedDisparity.h"
typedef unsigned int u_int;

/* This functions takes absolute path of one image pair
 * if one or two image file is not found return false
 * else return true, which means images are read correctly
 * file1: left_image
 * file2: right_image
 */
bool Disparity::ReadImagePair(char *file1, char *file2){
    img_left = imread(file1, CV_LOAD_IMAGE_GRAYSCALE);
    img_right = imread(file2, CV_LOAD_IMAGE_GRAYSCALE);

    if(!img_left.data){
        printf("Image %s not found",file1);
        return false;
    }
    if(!img_right.data){
        printf("Image %s not found",file2);
        return false;
    }
    return true;
}

/* This function return the disparity map after a stereo pair is loaded
 * loaded into the class instance. Provided both-direction block matching
 * result
 * subpixel: if generated disparity need sub-pixel accuracy
 * occlusion: if disparity need double direction occlusion test
 *           False: default will be Left direction
 *           True: double direction matching, the threshold is 0.5 pixel
 * ct: specify the algorithm for cost function, default is SSD
 */

Mat Disparity::FindDisparity(bool subpixel, bool occlusion, CostType ct){
    if(!occlusion){
        Mat raw = FindDisparityRaw(subpixel,ct,Left);
        return NormalizeRaw(raw);
    }

    float threshold = 1.0f;
    Mat left_disp = FindDisparityRaw(subpixel, ct, Left);
    Mat right_disp = FindDisparityRaw(subpixel, ct, Right);
    int rows = left_disp.rows;
    int cols = left_disp.cols;
#pragma omp parallel for
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
           float base_disf = left_disp.at<float>(r,c);
           int base_dis1 = (int)base_disf;
           int base_dis2 = base_dis1 + 1;
           if(c-base_dis2 < 0){
               left_disp.at<float>(r,c) = 0.0f;
               continue;
           }
           float back_dis1 = right_disp.at<float>(r,c-base_dis1);
           float back_dis2 = right_disp.at<float>(r,c-base_dis2);
           float back_disf = (base_dis2 - base_disf)*back_dis2 + (base_disf - base_dis1) * back_dis1;
           float val = abs(base_disf - back_disf);
           if(val > threshold){
               left_disp.at<float>(r,c) = 0.0f;
           }
        }
    }
    return NormalizeRaw(left_disp);
}

/* This function return the disparity map after a stereo pair is loaded
 * loaded into the class instance. Provided single-direction block matching
 * result
 * subpixel: if generated disparity need sub-pixel accuracy
 *           given three points (left, middle, right) points of parabola, x as
 *           disparity value in pixel, y as cost value, find the point (x value)
 *           on the parabola where y is the minimum
 * ct: specify the algorithm for cost function, default is SSD
 * direct: when do block matching, which direction to search
 *         if Left: based on left image, search blocks on left-ward on right image
 *         if Right: based on righ image, search blocks on right-ward on left image
 */
Mat Disparity::FindDisparityRaw(bool sub_pixel, CostType ct, Direction direct){
    Mat img1, img2;
    if(direct == Left){
        img1 = img_left; //base image
        img2 = img_right; //matching image
    }else{
        img1 = img_right; //base image
        img2 = img_left; //matching image
    }
    if(!img1.data || !img2.data){
        printf("Error: image data is invalid, unable to calculate disparity\n");
        return Mat(100,100,CV_32FC1);
    }

    int range = this->max_range;
    int rows = img1.rows;
    int cols = img1.cols;
    int r = patch_size/2;
    Mat disparity(rows, cols, CV_32FC1);
#pragma omp parallel for
    for(int y = 0; y < rows; y++){
        for(int x = 0; x < cols; x++){
            //printf("iteration %d\n", y*cols + x);
            float min_cost = INT_MAX;
            float distance = 0.0;

            int left_bound = (x-r >= 0)? x-r:0;
            int right_bound = (x+r < cols)? x+r:cols-1;
            int up_bound = (y-r >= 0)? y-r: 0;
            int bottom_bound = (y+r < rows)? y+r:rows-1;

            int width = right_bound - left_bound + 1;
            int height = bottom_bound - up_bound + 1;
            //printf("patch data: %d %d %d %d\n", left_bound, up_bound, width, height);
            Rect rect1(left_bound, up_bound, width, height);
            Mat patch1(img1,rect1);
            //printf("FindDisparityRaw(bool sub_pixel, CostType ct, Directionget pacth successfully\n");

            for(int i = 0; i < range; i++){
                int sign = 1;
                if(direct == Right){
                    sign = -1;
                }

                int left_bound2 = left_bound - sign * i;
                int right_bound2 = right_bound - sign * i;
                if(left_bound2 < 0 || right_bound2 >= cols){
                    continue;
                }
                Rect rect2(left_bound2, up_bound, width, height);
                Mat patch2(img2, rect2);

                float cost;
                FuncSelector(ct,cost,patch1,patch2);
                if(cost < min_cost){
                    min_cost = cost;
                    distance = (float)i;
                    //adding sub_pixel function
                    if(sub_pixel){
                        //cannot be edge
                        if(left_bound2 != 0 && right_bound2 != cols-1){
                            Rect rectleft(left_bound2-1, up_bound, width, height);
                            Rect rectright(left_bound2+1, up_bound, width, height);
                            Mat patch_left(img2,rectleft);
                            Mat patch_right(img2, rectright);
                            float y1, y3;
                            FuncSelector(ct,y1,patch1,patch_left);
                            FuncSelector(ct,y3,patch1,patch_right);
                            //distance -= sign*0.5*(costl - costr)/(costl - 2 * cost + costr);

                            float y2 = cost;
                            float x2 = distance;
                            float x1 = distance + sign*1;
                            float x3 = distance - sign*1;
                            float a = (((y2-y1)/(x2-x1))-((y3-y2)/(x3-x2)))/(x1-x3);
                            float b = (y2-y1)/(x2-x1)- a *(x2+x1);
                            distance = -b/(2*a);
                            //printf("subpixel\n");
                        }
                    }
                }

            }

            disparity.at<float>(y,x) = distance;
        }
    }
    return disparity;
}

/* This functions select differet cost function to calculate cost
 * based on the enum value given by the argument
 * Support Cost Function: SSD, NCC, ZNCC
 */
void Disparity::FuncSelector(CostType ct, float &cost, Mat patch1, Mat patch2){
    switch(ct)
    {
    case SSD: cost = SSDOfPatches(patch1, patch2); break;
    case NCC: cost = OneSubNCCOfPatches(patch1,patch2);break;
    case ZNCC: cost = OneSubZNCCOfPatches(patch1, patch2);break;
    default: cost = SSDOfPatches(patch1,patch2);break;
    }
    return;
}

/* This function returns integer value of
 * sum of squared difference between image patches
 */
float Disparity::SSDOfPatches(Mat patch1, Mat patch2){
    //check for validity of data
    if(!patch1.data || !patch2.data){
        printf("Error: image patch's data is invalid\n");
        return INT_MAX;
    }

    float ssd = 0;
    int rows = patch1.rows;
    int cols = patch1.cols;
    if(rows != patch2.rows || cols != patch2.cols){
        printf("Error: two image patches have differet size\n");
        return INT_MAX;
    }

    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            int p1 = patch1.at<uchar>(r,c);
            int p2 = patch2.at<uchar>(r,c);
            ssd += (p1-p2) * (p1-p2);
        }
    }
    return ssd;
}

/* This function return the one minus zero normalized cross-correlations of
 * two image patches
 */
float Disparity::OneSubZNCCOfPatches(Mat patch1, Mat patch2){
    //check for validity of data
    if(!patch1.data || !patch2.data){
        printf("Error: image patch's data is invalid\n");
        return 2.0;
    }

    int ncc = 0;
    int rows = patch1.rows;
    int cols = patch1.cols;
    if(rows != patch2.rows || cols != patch2.cols){
        printf("Error: two image patches have differet size\n");
        return 2.0;
    }

    //average pixel value of both patches
    int psum1 = 0;
    int psum2 = 0;
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            psum1 += patch1.at<uchar>(r,c);\
            psum2 += patch2.at<uchar>(r,c);
        }
    }
    float pavg1 = (float)psum1/(float)(rows*cols);
    float pavg2 = (float)psum2/(float)(rows*cols);

    //three variables for ncc
    float sumofmulti = 0.0;
    float sumofsquared1 = 0.0;
    float sumofsquared2 = 0.0;
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            int p1 = patch1.at<uchar>(r,c);
            int p2 = patch2.at<uchar>(r,c);
            float v1 = (float)p1-pavg1;
            float v2 = (float)p2-pavg2;
            sumofsquared1 += (v1*v1);
            sumofsquared2 += (v2*v2);
            sumofmulti += v1*v2;
        }
    }
    float res = sumofmulti/(pow(sumofsquared1,0.5)*pow(sumofsquared2,0.5));
    return 1 - res;
}

/* This function return the one minus normalized cross-correlations of
 * two image patches
 */
float Disparity::OneSubNCCOfPatches(Mat patch1, Mat patch2){
    //check for validity of data
    if(!patch1.data || !patch2.data){
        printf("Error: image patch's data is invalid\n");
        return 2.0;
    }

    int ncc = 0;
    int rows = patch1.rows;
    int cols = patch1.cols;
    if(rows != patch2.rows || cols != patch2.cols){
        printf("Error: two image patches have differet size\n");
        return 2.0;
    }

    //three variables for ncc
    float sumofmulti = 0.0;
    float sumofsquared1 = 0.0;
    float sumofsquared2 = 0.0;
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            int p1 = patch1.at<uchar>(r,c);
            int p2 = patch2.at<uchar>(r,c);
            float v1 = (float)p1;
            float v2 = (float)p2;
            sumofsquared1 += (v1*v1);
            sumofsquared2 += (v2*v2);
            sumofmulti += v1*v2;
        }
    }
    float res = sumofmulti/(pow(sumofsquared1,0.5)*pow(sumofsquared2,0.5));
    return 1 - res;
}

/* This function returns a uchar Mat type converted from a given
 * raw Mat type, it also normalize the pixel value to the max_range
 */
Mat Disparity::NormalizeRaw(Mat raw){
    if(!raw.data){
        return Mat(100,100,CV_8UC1);
    }
    int rows = raw.rows;
    int cols = raw.cols;
    Mat rt(rows, cols, CV_8UC1);
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            rt.at<uchar>(r,c) = (int)((raw.at<float>(r,c)/(float)max_range)*256);

        }
    }
    return rt;

}


