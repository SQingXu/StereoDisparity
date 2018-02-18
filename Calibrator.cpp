#include "Calibrator.h"

Calibrator::Calibrator(){

}

Calibrator::~Calibrator(){

}

void Calibrator::LoadCalibInfo(Size boaSize, float sqrSize, String path,
                          vector<unsigned int> indexes)
{
    boardSize.width = boaSize.width;
    boardSize.height = boaSize.height;
    squareSize = sqrSize;
    LoadImages(path,indexes);
}

void Calibrator::LoadImages(String path, vector<unsigned int> indexes){
    DFSVPlayer player;
    player.Open(path);
    num_streams = player.GetNumStreams();
    stream_packets.resize(num_streams);
    cameraMatrixes.resize(num_streams);
    distCoeffses.resize(num_streams);
    rvecses.resize(num_streams);
    tvecses.resize(num_streams);
    reprojErrses.resize(num_streams);
    totalAvgErrs.resize(num_streams);

    num_frames = indexes.size();
    imageSize.width = player.GetStreamInfo().w;
    imageSize.height = player.GetStreamInfo().h;

    //resize the image list's size and load images
    image_buffers.resize(num_frames);
    for(unsigned int i = 0; i < num_frames; i++){
        image_buffers[i].resize(num_streams);

        unsigned int index = indexes[i];
        player.SetCurrentFrameNumber(index);
        player.GrabNextFrame(stream_packets);
        for(unsigned int s = 0; s < num_streams; s++){
            stream_packets[s].image_buffer.copyTo(image_buffers[i][s]);
        }
    }
    player.Close();
}

void Calibrator::IntrisicCalibration(){
    namedWindow("Image Preview", WINDOW_AUTOSIZE);

    int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;
    chessBoardFlags |= CALIB_CB_FAST_CHECK;

    if(num_streams != 2){
        std::cout << "Not Stereo Setup" << std::endl;
        return;
    }
    image_points.resize(num_streams);
    unsigned int pro_image_pair = 0;
    for(unsigned int i = 0; i < num_frames; i++){
        //for each frame, both streams must find patterns
        vector<vector<Point2f>> pointBuf;
        //for pointBuf, first deimension is stream, second dimension is points
        pointBuf.resize(2);
        bool found_all = false;
        for(unsigned int s = 0; s < num_streams; s++){
            Mat image = image_buffers[i][s];
            Mat eightBitImage;
            ConvertGray16ToGray8(image,eightBitImage);
            imshow("Image Preview", eightBitImage);
            bool found = findChessboardCorners(eightBitImage,boardSize,
            pointBuf[s],CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
            if(found){
                //image already Gray

                cornerSubPix(eightBitImage, pointBuf[s], Size(11,11), Size(-1,-1),
                TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));
                drawChessboardCorners(image,boardSize,Mat(pointBuf[s]),found);
                found_all = true;
            }else{
                std::cout << "Corner not found " << i << " " << s << std::endl;
                found_all = false;
                break;
            }
        }
        if(found_all){
            pro_image_pair++;
            for(int n = 0; n < num_streams; n++){
                image_points[n].push_back(pointBuf[n]);
            }
            std::cout << "One pair detected, current pairs "
                      << pro_image_pair << std::endl;
            std::array<cv::Mat,2> raw_images = {
                image_buffers[i][0],
                image_buffers[i][1]
            };
            // concatenate for display purposes
            cv::Mat image_pair(raw_images[0].rows, 2 * raw_images[0].cols, CV_16UC1);
            cv::hconcat(raw_images[1], raw_images[0], image_pair);
            //std::cout << raw_images[0].rows << " " << raw_images[1].cols << std::endl;
            cv::imshow("Image Preview", image_pair);
            if(i == 0){
                Mat img8;
                ConvertGray16ToGray8(image_pair,img8);
                imwrite("depth_calib.jpg", img8);
            }
        }
    }
    std::cout << pro_image_pair << " image pairs are found proper" << std::endl;

    //After find image point, then calibrate
    for(int n = 0; n < num_streams; n++){
        cameraMatrixes[n] = Mat::eye(3,3,CV_64F);
        distCoeffses[n] = Mat::zeros(8,1,CV_64F);
    }
    objectPoints.resize(1);

    //calculate board corner position
    for(int i = 0; i < boardSize.height; i++){
        for(int j = 0; j < boardSize.width; j++){
            objectPoints[0].push_back(Point3f(j*squareSize, i*squareSize, 0));
        }
    }
    objectPoints.resize(image_points[0].size(),objectPoints[0]);
    //for both camera
    double rms;
    int flag = 0;
    flag |= CALIB_FIX_K4;
    flag |= CALIB_FIX_K5;
    for(int n = 0; n < num_streams; n++){
        rms = calibrateCamera(objectPoints, image_points[n],imageSize,
                              cameraMatrixes[n],distCoeffses[n],rvecses[n],
                              tvecses[n],flag);
        std::cout << "Re-projection error for stream " << n << " is "
                  << rms << std::endl;
        bool ok = checkRange(cameraMatrixes[n]) && checkRange(distCoeffses[n]);
        std::cout << "Result is " << ok << std::endl;
        totalAvgErrs[n] = computeReprojectionErrors(objectPoints,image_points[n],
                                  rvecses[n],tvecses[n],cameraMatrixes[n],
                                  distCoeffses[n],reprojErrses[n]);
        std::cout << "Camera Matrix: " << std::endl;
        debugPrintMatrix<double>(cameraMatrixes[n]);
        std::cout << "Distortion Coefficient: " << std::endl;
        debugPrintMatrix<double>(distCoeffses[n]);

    }


}

void Calibrator::StereoExtrinsicCalibration(){
    stereoCalibrate(objectPoints, image_points[0],image_points[1],
            cameraMatrixes[0], distCoeffses[0],cameraMatrixes[1], distCoeffses[1],
            imageSize, R,T,E,F);
    std::cout << "Rotation Matrix: " << std::endl;
    debugPrintMatrix<double>(R);
    std::cout << "Translatin Matrix: " << std::endl;
    debugPrintMatrix<double>(T);
}

double Calibrator::computeReprojectionErrors(const vector<vector<Point3f> > &objectPoints,
                                             const vector<vector<Point2f> > &imagePoints,
                                             const vector<Mat> &rvecs, const vector<Mat> &tvecs,
                                             const Mat &cameraMatrix, const Mat &distCoeffs,
                                             vector<float> &perViewErrors){
    vector<Point2f> imagePoints2;
    size_t totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for(size_t i = 0; i < objectPoints.size(); ++i )
    {
        projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
        err = norm(imagePoints[i], imagePoints2, NORM_L2);

        size_t n = objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

void Calibrator::ConvertGray16ToGray8(Mat sixteenBit, Mat &eightBit){
    eightBit = Mat(sixteenBit.rows, sixteenBit.cols, CV_8UC1);
    for(int r = 0; r < sixteenBit.rows;r++){
        for(int c = 0; c < sixteenBit.cols; c++){
            eightBit.at<uchar>(r,c) = (uchar)(((int)sixteenBit.at<short>(r,c)) >> 8);
        }
    }
}

void Calibrator::SaveCalibration(){
    for(int n = 0; n < num_streams; n++){
        ofstream outfile;
        char calibStr[100];

        sprintf(calibStr, "/playpen/StereoDisparity/Capture/calibrationResult%d.txt",n+1);
        outfile.open(calibStr,std::ofstream::out | std::ofstream::app);
        outfile << imageSize.width << " " << imageSize.height << std::endl;
        writeMatrix<double>(cameraMatrixes[n],outfile);
        Mat rot, trans;

        if(n == 0){
            rot = Mat::eye(3,3,CV_64F);
            trans = Mat::zeros(3,1,CV_64F);
        }else{
            rot = R;
            trans = T;
        }
        if(!R.empty() && !T.empty()){
            writeMatrix<double>(rot,outfile);
            writeMatrix<double>(trans, outfile);
            writeMatrix<double>(distCoeffses[n], outfile);
        }else{
            std::cout << "Invalid value in Matrix" << std::endl;
        }
        outfile.close();
    }
}

template <typename T>void Calibrator::writeMatrix(Mat mat, ofstream& os){
    for(int r = 0; r < mat.rows; r++){
        for(int c = 0; c < mat.cols; c++){
            os << mat.at<T>(r,c);
            if(c != mat.cols - 1){
                os << " ";
            }
        }
        os << std::endl;
    }
}

template <typename T>void Calibrator::debugPrintMatrix(Mat mat){
    for(int r = 0; r < mat.rows; r++){
        for(int c = 0; c < mat.cols; c++){
            std::cout << mat.at<T>(r,c) << " ";
        }
        std::cout << std::endl;
    }
}
