#ifndef FLYCAPTUREPLAYER_H
#define FLYCAPTUREPLAYER_H

#endif // FLYCAPTUREPLAYER_H

#include "flycapture/FlyCapture2.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <memory>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;

//define error checking macro
#define FLY_CAPTURE_ASSERT(FLY_FUNC) \
{                                    \
    FlyCapture2::Error error = FLY_FUNC;   \
    if(error != FlyCapture2::PGRERROR_OK){ \
        error.PrintErrorTrace();           \
        printf("Exiting due to error");    \
    }                                      \
}                                          \


//create struct for saving frame's info and data
struct TimeStamp
{
    /** Seconds. */
    double seconds;
    /** Microseconds. */
    size_t microSeconds;
};

struct StreamInfo
{
    unsigned int w;
    unsigned int h;
    //SImageFormat fmt;
    size_t cvfmt;
};

struct StreamPacket
{

    Mat image_buffer;

    TimeStamp tStamp;

    StreamPacket(){}

    // Not copy constructable
    inline
    StreamPacket( const StreamPacket& other ) = delete;

    // Move constructor
    inline
    StreamPacket(StreamPacket&& s)
    {
        *this = std::move(s);
    }

    // Move assignment
    inline
    void operator=(StreamPacket&& s)
    {
        this->tStamp = s.tStamp;
        this->image_buffer = std::move(s.image_buffer);
    }

    void CopyFrom(const StreamPacket& s)
    {
        this->tStamp = s.tStamp;

        //SImageFormat fmt = s.image_buffer.fmt;
        //this->image_buffer.Reinitialise(s.image_buffer.w,s.image_buffer.h,s.image_buffer.fmt);
        //this->image_buffer.CopyFrom(s.image_buffer);
        s.image_buffer.copyTo(this->image_buffer);
        //this->image_buffer.SetFmt(fmt);
    }
};

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
