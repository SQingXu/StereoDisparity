#include "PLYConverter.h"

PLYConverter::PLYConverter(){

}

PLYConverter::~PLYConverter(){

}

void PLYConverter::writeDepthToPLY(const char *path, Mat depth, Camera cam){
    FILE *outPly;
    outPly = fopen(path, "w+");
    fprintf(outPly,"ply\n");
    fprintf(outPly,"format ascii 1.0\n");
    fprintf(outPly,"comment This contains a Splatted Point Cloud\n");
    fprintf(outPly,"element vertex %d\n",depth.rows*depth.cols);
    fprintf(outPly,"property float x\n");
    fprintf(outPly,"property float y\n");
    fprintf(outPly,"property float z\n");
    fprintf(outPly,"element face %d\n",0);
    fprintf(outPly,"property list uchar int vertex_indices\n");
    fprintf(outPly,"end_header\n");
    for(int r = 0; r < depth.rows; r++){
        for(int c = 0; c < depth.cols; c++){
            Mat world_cord = Mat(3,1,CV_32FC1);
            Mat pixel_cord = Mat(2,1,CV_32FC1);
            pixel_cord.at<float>(0,0) = (float)c;//x
            pixel_cord.at<float>(1,0) = (float)r;//y
            cam.unprojectPt(pixel_cord, world_cord, depth.at<float>(r,c));
            vector<float> w_v;
            w_v.push_back(world_cord.at<float>(0,0));
            w_v.push_back(world_cord.at<float>(1,0));
            w_v.push_back(world_cord.at<float>(2,0));
            fprintf(outPly, "%8f %8f %8f\n",w_v[0],w_v[1], w_v[2]);
        }
    }
    fclose(outPly);

}



