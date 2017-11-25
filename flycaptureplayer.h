#pragma once
#include "StreamUtil.h"

using namespace std;
using namespace cv;



class FlyCapturePlayer
{
public:
    FlyCapturePlayer();
    ~FlyCapturePlayer();
    void Open();
    void Start();
    void Stop();
    void StartRecord(String out_path);
    void WriteStreams(vector<StreamPacket>& streams);
    void StopRecord();
    void GrabNextFrame(vector<StreamPacket>& streams);
private:
    //for rendering
    unsigned int num_streams;
    size_t size_bytes;
    unsigned int current_frame;
    FlyCapture2::BusManager bus_manager;
    vector<std::unique_ptr<FlyCapture2::Camera>> cameras;
    vector<FlyCapture2::Image> raw_images, conv_images;

    vector<StreamPacket> streamPackets;
    vector<StreamInfo> streamInfos;
    vector<FlyCapture2::PixelFormat> conv_fmt;
    vector<double> time_stamps_sec;
    vector<unsigned int> time_stamps_temp;
    bool getImageFormat(const FlyCapture2::PixelFormat& fm,size_t& s_fm, FlyCapture2::PixelFormat& conv);
    //for recording
    ofstream out_file;

    vector<unsigned int>image_buffer_size;
    vector<unsigned int>image_meta_buffer_size;
    unsigned int total_size;
    unsigned int num_frames;

};
