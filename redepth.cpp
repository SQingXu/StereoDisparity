#include "redepth.h"

/* This functions takes absolute path of one image pair
 * if one or two image file is not found return false
 * else return true, which means images are read correctly
 */
bool ReverseDepth::ReadImagePair(char *file1, char *file2, char* calib1, char* calib2){
    img_a = imread(file1, CV_LOAD_IMAGE_GRAYSCALE);
    img_b = imread(file2, CV_LOAD_IMAGE_GRAYSCALE);

    if(!img_a.data){
        printf("Image %s not found\n",file1);
        return false;
    }
    if(!img_b.data){
        printf("Image %s not found\n",file2);
        return false;
    }
    c1 = Camera();
    c2 = Camera();
    if(!c1.readCalibration(calib1)){
        printf("Calibration file %s not found\n",calib1);
    }
    if(!c2.readCalibration(calib2)){
        printf("Calibration file %s not found\n",calib2);
    }
    printf("Read data successfully\n");
    return true;
}

Mat ReverseDepth::FindDepth(bool subpixel, bool occlusion, CostType ct){
    if(!occlusion){
        Mat raw = FindDepthRaw(subpixel, ct, TwoToOne);
        return NormalizeRaw(raw);
    }
    Mat raw_12 = FindDepthRaw(subpixel,ct,OneToTwo);
    Mat raw_21 = FindDepthRaw(subpixel,ct,TwoToOne);
    float threshold = 5.0f;
    int rows = raw_12.rows;
    int cols = raw_12.cols;
    float step = (1/range_min - 1/range_max)/(float)max_iteration;
#pragma omp parallel for
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            float base_depth = raw_12.at<float>(r,c);
            //with given depth project this point onto depth map 2
            Mat pixel_cord1(2,1,CV_32FC1);
            Mat pixel_cord2(2,1,CV_32FC1);
            Mat point_3d(3,1,CV_32FC1);
            Mat point_c2(3,1,CV_32FC1);
            pixel_cord1.at<float>(0,0) = (float)c;//x
            pixel_cord1.at<float>(1,0) = (float)r;//y
            if(!c1.unprojectPt(pixel_cord1,point_3d,base_depth)){
                continue;
            }
            if(!c2.projectPt(point_3d, pixel_cord2)){
                continue;
            }
            c2.worldToCamPt(point_3d,point_c2);
            float x_f = pixel_cord2.at<float>(0,0);
            float y_f = pixel_cord2.at<float>(1,0);
            if(x_f < 0 || x_f >= cols-1 || y_f < 0 || y_f >= rows-1){
                raw_12.at<float>(r,c) = 0.0f;
                continue;
            }
            int x1 = (int)x_f;
            int x2 = ((int)x_f) + 1;
            int y1 = (int)y_f;
            int y2 = ((int)y_f) + 1;
            if(x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0){
                //printf("xf: %5f, x1: %d, x2: %d, y1: %d, y2: %d\n",x_f,x1, x2, y1, y2);
                raw_12.at<float>(r,c) = 0.0f;
                continue;
            }
            float v11 = raw_21.at<float>(y1,x1);
            float v21 = raw_21.at<float>(y1,x2);
            float v12 = raw_21.at<float>(y2,x1);
            float v22 = raw_21.at<float>(y2,x2);
            float x1_p = ((float)x2 - x_f)/(x2 - x1);
            float x2_p = ((float)x_f - x1)/(x2 - x1);
            float y1_p = ((float)y2 - y_f)/(y2 - y1);
            float y2_p = ((float)y_f - y1)/(y2 - y1);
            float back_depth = (y1_p*(x1_p*v11 + x2_p*v21) + y2_p*(x1_p*v12 + x2_p*v22));

            float val = abs((float)1/point_c2.at<float>(2,0) - (float)1/back_depth)/step;
            if(val > threshold){
                raw_12.at<float>(r,c) = 0.0f;
            }
        }
    }
    PLYConverter ply_converter;
    ply_converter.writeDepthToPLY("/playpen/StereoDisparity/Capture/output_nosub.ply",raw_12,c1);
    return NormalizeRaw(raw_12);


}

void ReverseDepth::SetRange(float min, float max){
    range_max = max;
    range_min = min;
    return;
}

Mat ReverseDepth::FindDepthRaw(bool subpixel, CostType ct, Direction direct){
    Mat img1, img2;
    Camera cam1, cam2;
    if(direct == OneToTwo){
        img1 = img_a;
        cam1 = c1;
        img2 = img_b;
        cam2 = c2;
    }else{
        img1 = img_b;
        cam1 = c2;
        img2 = img_a;
        cam2 = c1;
    }
    if(!img1.data || !img2.data){
        printf("image data is invalid\n");
        return Mat(100,100,CV_32FC1);
    }

    int rows = img1.rows;
    int cols = img1.cols;
    int range = max_iteration;
    int r = patch_size/2;
    Mat disparity(rows, cols, CV_32FC1);
    float step = (1/range_min - 1/range_max)/(float)range;
    float base = 1/range_min;
#pragma omp parallel for
    for(int y = 0; y < rows; y++){
        for(int x = 0; x < cols; x++){
            float min_cost = INT_MAX;
            float best_depth = 0.0;

            int left_bound = (x-r >= 0)? x-r:0;
            int right_bound = (x+r < cols)? x+r:cols-1;
            int up_bound = (y-r >= 0)? y-r: 0;
            int bottom_bound = (y+r < rows)? y+r:rows-1;

            int width_left = x - left_bound;
            int width_right = right_bound - x;
            int height_down = bottom_bound - y;
            int height_up = y - up_bound;


            Rect rect1(left_bound, up_bound, width_left + width_right + 1, height_down + height_up + 1);
            Mat patch1(img1,rect1);
            Mat patch2(height_down + height_up + 1, width_left + width_right + 1, CV_8UC1);
            Mat in(2,1,CV_32FC1);
            in.at<float>(0,0) = (float)x;
            in.at<float>(1,0) = (float)y;
            Mat mid(3,1,CV_32FC1);
            Mat res(2,1,CV_32FC1);
            float y1 = 0, y2 = 0, y3 = 0;
            float pre_cost = 0;
            float best_i = 0;
            bool find_min= false;
            for(int i = 0; i < range; i++){
                float depth = 1/(base - i * step);
                //printf("depth: %5f, step: %5f\n",depth, step);
                //first project point from img1 to img2


                if(!cam1.unprojectPt(in,mid, depth)){
                    continue;
                }
                if(!cam2.projectPt(mid,res)){
                    continue;
                }
                GetBilinearPatch(res.at<float>(0,0), res.at<float>(1,0),width_left, width_right,height_up,height_down,img2,patch2);
                if(patch2.empty()){
                    continue;
                }

                float cost;
                FuncSelector(ct, cost, patch1, patch2);

                if(subpixel){
                    if(find_min){
                        y3 = cost;
                        find_min = false;
                    }
                    if(cost < min_cost){
                        min_cost = cost;
                        //best_depth = depth;
                        y1 = pre_cost;
                        best_i = (float)i;
                        find_min = true;
                    }
                    //update previous cost only at the end of the loop
                    pre_cost = cost;
                }else{
                    if(cost < min_cost){
                        min_cost = cost;
                        best_depth = depth;
                    }
                }

            }
            if(subpixel){
                if(best_i != 0 && best_i != range-1){
                    y2 = min_cost;
                    float x1 = (float)(best_i - 1);
                    float x2 = (float)best_i;
                    float x3 = (float)(best_i + 1);
                    float a = (((y2-y1)/(x2-x1))-((y3-y2)/(x3-x2)))/(x1-x3);
                    float b = (y2-y1)/(x2-x1)- a *(x2+x1);
                    float c = y2 - (x2*x2*a) - (b * x2);
                    float new_i = -b/(2*a);
                    //min_cost = (new_i*new_i*a) + (new_i*b) + c;
                    best_depth = 1/(base - new_i * step);
                }
            }
            disparity.at<float>(y,x) = best_depth;
            //printf("x: %d y: %d depth: %5f\n",x,y,best_depth);

        }
    }
    return disparity;

}

/* This functions select differet cost function to calculate cost
 * based on the enum value given by the argument
 * Support Cost Function: SSD, NCC, ZNCC
 */
void ReverseDepth::FuncSelector(CostType ct, float &cost, Mat patch1, Mat patch2){
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
float ReverseDepth::SSDOfPatches(Mat patch1, Mat patch2){
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
float ReverseDepth::OneSubZNCCOfPatches(Mat patch1, Mat patch2){
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
float ReverseDepth::OneSubNCCOfPatches(Mat patch1, Mat patch2){
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
Mat ReverseDepth::NormalizeRaw(Mat raw){
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
                rt.at<uchar>(r,c) = 0;
                continue;
            }
            rt.at<uchar>(r,c) = (int)((val/range_max)*256);
        }
    }
    return rt;

}

bool ReverseDepth::GetBilinearPatch(float x, float y, int w_l, int w_r, int h_u, int h_d, Mat img,Mat& patch){
    if(x - w_l < 0 || x + w_r >= img.cols - 1 || y - h_u < 0 || y + h_d >= img.rows -1){
        return false;
    }
    //Mat patch(h_u + h_d + 1, w_l + w_r + 1, CV_8UC1);
    int p_x = 0, p_y = 0;
    for(int r = -h_u; r <= h_d; r++){
        for(int c = -w_l; c <= w_r; c++){
            float x_f = x + c;
            float y_f = y + r;
            int x1 = (int)x_f;
            int x2 = ((int)x_f) + 1;
            int y1 = (int)y_f;
            int y2 = ((int)y_f)+1;
            int v11 = img.at<uchar>(y1,x1);
            int v21 = img.at<uchar>(y1,x2);
            int v12 = img.at<uchar>(y2,x1);
            int v22 = img.at<uchar>(y2,x2);
            float x1_p = ((float)x2 - x_f)/(x2 - x1);
            float x2_p = ((float)x_f - x1)/(x2 - x1);
            float y1_p = ((float)y2 - y_f)/(y2 - y1);
            float y2_p = ((float)y_f - y1)/(y2 - y1);
            int v = (int)(y1_p*(x1_p*v11 + x2_p*v21) + y2_p*(x1_p*v12 + x2_p*v22));
            patch.at<uchar>(p_y, p_x) = v;
            p_x = (p_x == (w_l+w_r))?0:p_x+1;
        }
        p_y++;
    }
    return true;
}

