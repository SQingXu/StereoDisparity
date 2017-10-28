#include "camera.h"

bool Camera::readCalibration(char *filename)
{
    ifstream cal_file;
    cal_file.open(filename);
    if(!cal_file)
    {
        printf("unable to open file %s", filename);
        return false;
    }

    //width and height
    cal_file >> this->width;
    cal_file >> this->height;

    //K matrix
    float k = 0.0;
    int x,y;
    K = Mat(3,3, CV_32FC1);
    K_inv = Mat(3,3,CV_32FC1);
    for(y = 0; y < 3; y++){
        for(x = 0; x < 3; x++){
            cal_file >> k;
            K.at<float>(y,x) = k;
        }
    }

    //rotation matrix
    E = Mat(4,4, CV_32FC1);
    R = Mat(3,3, CV_32FC1);
    for(y = 0; y < 3; y++){
        for(x = 0; x < 3; x++){
            cal_file >> k;
            R.at<float>(y,x) = k;
            E.at<float>(y,x) = k;
        }
    }

    //translation
    t = Mat(1,3, CV_32FC1);
    for(x = 0; x < 3; x++){
        cal_file >> k;
        t.at<float>(0,x) = k;
        E.at<float>(x,3) = k;
    }

    //last row of E
    for(x = 0; x < 4; x++){
        E.at<float>(3,x) = (x == 3)?1:0;
    }
    E_inv = E.inv();

    //distortion coefficient
    dist = Mat(1,5, CV_32FC1);
    for(x = 0; x < 5; x++){
        cal_file >> k;
        dist.at<float>(0,x) = k;
    }
    k1 = dist.at<float>(0,0);
    k2 = dist.at<float>(0,1);
    p1 = dist.at<float>(0,2);
    p2 = dist.at<float>(0,3);
    k3 = dist.at<float>(0,4);
    return true;
}

bool Camera::projectPt(Mat point_3d, Mat& pixel_cord)
{
    //3D to 2D
    Mat homo_pt(4,1, CV_32FC1);
    homo_pt.at<float>(0,0) = point_3d.at<float>(0,0);
    homo_pt.at<float>(1,0) = point_3d.at<float>(1,0);
    homo_pt.at<float>(2,0) = point_3d.at<float>(2,0);
    homo_pt.at<float>(3,0) = 1;
    homo_pt = E * homo_pt;
    //normalize
    Mat normal_pt(3,1,CV_32FC1);
    float z = homo_pt.at<float>(2,0);
    normal_pt.at<float>(0,0) = homo_pt.at<float>(0,0)/z;
    normal_pt.at<float>(1,0) = homo_pt.at<float>(1,0)/z;
    normal_pt.at<float>(2,0) = 1;
    printf("before distort: %5f, %5f, %5f\n",normal_pt.at<float>(0,0), normal_pt.at<float>(1,0), normal_pt.at<float>(2,0));
    distortPt(normal_pt, normal_pt);
    printf("distort: %5f, %5f, %5f\n",normal_pt.at<float>(0,0), normal_pt.at<float>(1,0), normal_pt.at<float>(2,0));
    normal_pt = K * normal_pt;

    pixel_cord.at<float>(0,0) = normal_pt.at<float>(0,0);
    pixel_cord.at<float>(1,0) = normal_pt.at<float>(1,0);
    return true;
}

bool Camera::unprojectPt(Mat pixel_cord, Mat& point_3d, float depth)
{
    //2D to 3D
    undistortPt(pixel_cord, point_3d);
    Mat homo_pt(4,1, CV_32FC1);
    homo_pt.at<float>(0,0) = point_3d.at<float>(0,0) * depth;
    homo_pt.at<float>(1,0) = point_3d.at<float>(1,0) * depth;
    homo_pt.at<float>(2,0) = depth;
    homo_pt.at<float>(3,0) = 1.0;
    homo_pt = E_inv * homo_pt;
    point_3d.at<float>(0,0) = homo_pt.at<float>(0,0);
    point_3d.at<float>(1,0) = homo_pt.at<float>(1,0);
    point_3d.at<float>(2,0) = homo_pt.at<float>(2,0);
    return true;
}

/* This function distort point (from 3D to 3D)
   using given distortion coefficient k1, k2, k3
   p1, p2 (radial distortion and tangential distortion)
   It takes normalized camera coordinates
 */
bool Camera::distortPt(Mat orig, Mat& dis)
{
    //2D to 2D
    float u = orig.at<float>(0,0);
    float v = orig.at<float>(1,0);
    float r_sqr = u * u + v * v;
    float radial_val = k1 * r_sqr + k2 * r_sqr * r_sqr + k3 * r_sqr * r_sqr * r_sqr;
    u = u + u * radial_val;
    v = v + v * radial_val;
    dis.at<float>(0,0) = u + (2*p1*u*v + p2 * (r_sqr + 2*u*u));
    dis.at<float>(1,0) = v + (p1 * (r_sqr + 2*v*v) + 2*p2*u*v);
    dis.at<float>(2,0) = 1;
    return true;
}

//given a image coordinates and returns 2D to 3D
bool Camera::undistortPt(Mat dis, Mat& orig)
{
    Mat dis_pts(1,1,CV_32FC2);
    dis_pts.at<Point2f>(0,0).x = dis.at<float>(0,0);
    dis_pts.at<Point2f>(0,0).y = dis.at<float>(1,0);
    Mat orig_pts(1,1,CV_32FC3);
    undistortPoints(dis_pts, orig_pts, K, dist);
    orig.at<float>(0,0) = orig_pts.at<Point3f>(0,0).x;
    orig.at<float>(1,0) = orig_pts.at<Point3f>(0,0).y;
    orig.at<float>(2,0) = orig_pts.at<Point3f>(0,0).z;
    return true;
}
