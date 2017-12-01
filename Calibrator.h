#pragma once
#include "DFSVPlayer.h"

class Calibrator{
public:
    Calibrator();
    ~Calibrator();

    void LoadCalibInfo(Size boaSize, float sqrSize,String path,
                       vector<unsigned int> indexes);
    void IntrisicCalibration();
    void StereoExtrinsicCalibration();
    void SaveCalibration();
    void ConvertGray16ToGray8(Mat sixteenBit, Mat& eightBit);
private:
    vector<vector<Mat>> image_buffers;//frist dimension set of image pair
    vector<vector<vector<Point2f>>> image_points;//first dimension stream
    vector<vector<Point3f>> objectPoints;
    vector<StreamPacket> stream_packets;
    vector<Mat> cameraMatrixes;
    vector<Mat> distCoeffses;
    vector<vector<Mat>> rvecses;
    vector<vector<Mat>> tvecses;
    vector<vector<float>> reprojErrses;
    vector<double> totalAvgErrs;
    Mat R, T, E, F;
    Size boardSize;
    Size imageSize;
    float squareSize;
    unsigned int num_streams;
    unsigned int num_frames;
    void LoadImages(String path, vector<unsigned int> indexes);
    double computeReprojectionErrors(const vector<vector<Point3f> >& objectPoints,
                                     const vector<vector<Point2f> >& imagePoints,
                                     const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                     const Mat& cameraMatrix , const Mat& distCoeffs,
                                     vector<float>& perViewErrors);
    template <typename T> void debugPrintMatrix(Mat mat);
    template <typename T> void writeMatrix(Mat mat, ofstream& os);
};
