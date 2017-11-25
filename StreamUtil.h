#pragma once

#include "flycapture/FlyCapture2.h"
#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <memory>
#include <vector>
#include <fstream>

//define error checking macro
#define FLY_CAPTURE_ASSERT(FLY_FUNC) \
{                                    \
    FlyCapture2::Error error = FLY_FUNC;   \
    if(error != FlyCapture2::PGRERROR_OK){ \
        error.PrintErrorTrace();           \
        printf("Exiting due to error");    \
    }                                      \
}                                          \

using namespace cv;
using namespace std;

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
