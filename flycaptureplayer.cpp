#include "flycaptureplayer.h"
#include <stdio.h>

FlyCapturePlayer::FlyCapturePlayer(){

}

FlyCapturePlayer::~FlyCapturePlayer(){

}

void FlyCapturePlayer::Open(){
    FlyCapture2::FC2Version fc2Version;
    FlyCapture2::Utilities::GetLibraryVersion(&fc2Version);

    std::cout << "FlyCapture2 library version" << fc2Version.major << "."
              << fc2Version.minor << "." << fc2Version.type << "."
              << fc2Version.build << endl;
    FLY_CAPTURE_ASSERT(bus_manager.GetNumOfCameras(&num_streams));

    printf("Number of camera detected %d\n", num_streams);

    streamPackets.resize(num_streams);
    raw_images.resize(num_streams);
    conv_images.resize(num_streams);
    cameras.resize(num_streams);
    streamInfos.resize(num_streams);
    conv_fmt.resize(num_streams);
    time_stamps_sec.resize(num_streams,0.0);
    time_stamps_temp.resize(num_streams,0);
    size_bytes = 0;

    for(unsigned int i = 0; i < num_streams; ++i){
    //for(int i = num_streams - 1; i >= 0; i--){
        FlyCapture2::PGRGuid guid;
        FLY_CAPTURE_ASSERT(bus_manager.GetCameraFromIndex(i,&guid));

        // Connect to the camera
        cameras[i].reset(new FlyCapture2::Camera);
        FLY_CAPTURE_ASSERT(cameras[i]->Connect(&guid));

        FlyCapture2::CameraInfo camInfo;
        FLY_CAPTURE_ASSERT(cameras[i]->GetCameraInfo(&camInfo));

        std::cout << std::endl
                  << "*** CAMERA INFORMATION ***" << std::endl
                  << "Serial number -" << camInfo.serialNumber << std::endl
                  << "Camera model - " << camInfo.modelName << std::endl
                  << "Camera vendor - " << camInfo.vendorName << std::endl
                  << "Sensor - " << camInfo.sensorInfo << std::endl
                  << "Resolution - " << camInfo.sensorResolution << std::endl
                  << "Firmware version - " << camInfo.firmwareVersion << std::endl
                  << "Firmware build time - " << camInfo.firmwareBuildTime<< std::endl
                  << "Bus Speed - " << camInfo.maximumBusSpeed<< std::endl
                  << std::endl
                  << std::endl;

        FlyCapture2::VideoMode videoMode;
        FlyCapture2::FrameRate frameRate;
        FLY_CAPTURE_ASSERT(cameras[i]->GetVideoModeAndFrameRate(&videoMode, &frameRate));

        bool format7 = false;
        FlyCapture2::Format7ImageSettings imageSettings;

        if(videoMode == FlyCapture2::VIDEOMODE_FORMAT7){
            format7 = true;

            unsigned int pSize;
            float percent;
            FLY_CAPTURE_ASSERT(cameras[i]->GetFormat7Configuration(&imageSettings, &pSize, &percent));

            std::cout << std::endl
                      << "*** VIDEO FORMAT_7 INFORMATION ***" << std::endl
                      << "OffsetX -" << imageSettings.offsetX << std::endl
                      << "OffsetY - " << imageSettings.offsetY << std::endl
                      << "Width -" << imageSettings.width << std::endl
                      << "Height - " << imageSettings.height << std::endl
                      << "PixelFormat - " << imageSettings.pixelFormat
                      << std::endl
                      << std::endl;
        }else{
            std::cout << std::endl
                      << "*** VIDEO INFORMATION ***" << std::endl
                      << "Video Mode -" << videoMode << std::endl
                      << "Frame Rate - " << frameRate
                      << std::endl
                      << std::endl;
        }

        FlyCapture2::FC2Config config;
        FLY_CAPTURE_ASSERT(cameras[i]->GetConfiguration(&config));
        std::cout << std::endl
                  << "*** VIDEO CONFIG ***" << std::endl
                  << "Sync Bus speed -" << config.isochBusSpeed << std::endl
                  << std::endl;

        if(format7 != true){
            printf("Format incorrect\n");
            return;
        }
        size_t cv_type;
        if(!getImageFormat(imageSettings.pixelFormat, cv_type, conv_fmt[i])){
            std::cout << std::endl
                      << "*** Image Format not supported ***" << std::endl;
            return;
        }

        streamInfos[i].w = imageSettings.width;
        streamInfos[i].h = imageSettings.height;
        streamInfos[i].cvfmt = cv_type;

        streamPackets[i].image_buffer = Mat(streamInfos[i].h, streamInfos[i].w,streamInfos[i].cvfmt);
        size_bytes += (streamPackets[i].image_buffer.step[0] * streamPackets[i].image_buffer.rows);

    }
}

void FlyCapturePlayer::Start(){
    FLY_CAPTURE_ASSERT(FlyCapture2::Camera::StartSyncCapture(
                           num_streams, (const FlyCapture2::Camera **)cameras.data()));
}

void FlyCapturePlayer::Stop(){
    for(auto &cam : cameras){
        FLY_CAPTURE_ASSERT(cam->StopCapture());
        FLY_CAPTURE_ASSERT(cam->Disconnect());
    }
}

void FlyCapturePlayer::GrabNextFrame(std::vector<StreamPacket>& streams){
    for(unsigned int i = 0; i < num_streams; ++i){
    //for(int i = num_streams-1; i >= 0; i--){
        FlyCapture2::TimeStamp t;
        FLY_CAPTURE_ASSERT(cameras[i]->RetrieveBuffer(&raw_images[i]));
        t = raw_images[i].GetTimeStamp();

        if(time_stamps_temp[i] != t.cycleSeconds){
            time_stamps_temp[i] = t.cycleSeconds;
            time_stamps_sec[i]++;
        }

        double time = time_stamps_sec[i] + (double)t.cycleCount/8000.0;
        if(conv_fmt[i] != FlyCapture2::UNSPECIFIED_PIXEL_FORMAT){
            FLY_CAPTURE_ASSERT(raw_images[i].Convert(conv_fmt[i], &conv_images[i]));
            //std::memcpy
            std::cout << "raw image size row: " << conv_images[i].GetRows() << " col: " << conv_images[i].GetCols() << std::endl;

            streamPackets[i].image_buffer = Mat(conv_images[i].GetRows(), conv_images[i].GetCols(), streamInfos[i].cvfmt);
            memcpy(streamPackets[i].image_buffer.data, conv_images[i].GetData(), conv_images[i].GetDataSize());
        }
        else{
            streamPackets[i].image_buffer = Mat(raw_images[i].GetRows(), raw_images[i].GetCols(), streamInfos[i].cvfmt);
            memcpy(streamPackets[i].image_buffer.data, raw_images[i].GetData(), raw_images[i].GetDataSize());
        }
        streamPackets[i].tStamp.microSeconds = (size_t)(time*1e6);
        streamPackets[i].tStamp.seconds = time;
        streams[i].CopyFrom(streamPackets[i]);
    }
}

void FlyCapturePlayer::StartRecord(String out_path){
    std::cout << "Start recording ..." << std::endl;
    out_file.open(out_path, std::ofstream::binary);
    if(!out_file.is_open()){
        std::cout << "File " << out_path << " cannot be opened" << std::endl;
        return;
    }
    image_buffer_size.resize(num_streams);
    image_meta_buffer_size.resize(num_streams);
    out_file.write((char *)&num_streams, sizeof(num_streams));

    total_size = sizeof(num_streams);
    num_frames = 0;
    for(unsigned int i = 0; i < num_streams; i++){
        image_buffer_size[i] = size_bytes/num_streams; //suppose every stream's frame have same size
        image_meta_buffer_size[i] = sizeof(size_t);
        out_file.write((char *)&streamInfos[i], sizeof(StreamInfo));

        total_size += sizeof(StreamInfo);
        std::cout << "Stream " << i+1 << "image size: " << image_buffer_size[i] << std::endl;
    }
}

void FlyCapturePlayer::StopRecord(){
    std::cout << "Stop recording " << num_frames << " in total" << std::endl;
    out_file.write((char *)&num_frames, sizeof(num_frames));
    out_file.close();
}

void FlyCapturePlayer::WriteStreams(vector<StreamPacket> &streams){
    num_frames++;
    for(unsigned int i = 0; i < num_streams; i++){
        out_file.write((char *)streams[i].image_buffer.data, image_buffer_size[i]);
        const TimeStamp& t = streams[i].tStamp;
        out_file.write((char *)&t.microSeconds, image_meta_buffer_size[i]);
        total_size += (image_buffer_size[i] + image_meta_buffer_size[i]);
    }
}

bool FlyCapturePlayer::getImageFormat(const FlyCapture2::PixelFormat& fm,size_t& s_fm,FlyCapture2::PixelFormat& conv)
{
    conv = FlyCapture2::UNSPECIFIED_PIXEL_FORMAT;
    switch(fm)
    {
        case FlyCapture2::PIXEL_FORMAT_MONO8:
            s_fm = CV_8UC1;
            return true;
        case FlyCapture2::PIXEL_FORMAT_RGB8:
            //s_fm = S_RGB24;
            s_fm = CV_8UC3;
            return true;
        case FlyCapture2::PIXEL_FORMAT_MONO16:
            //s_fm = S_GRAY16;
            s_fm = CV_16UC1;
            return true;
        case FlyCapture2::PIXEL_FORMAT_RAW8:
            //s_fm = S_RGB24;
            s_fm = CV_8UC3;
            conv = FlyCapture2::PIXEL_FORMAT_RGB8;
            return true;
        case FlyCapture2::PIXEL_FORMAT_411YUV8:
            //s_fm = S_RGB24;
            s_fm = CV_8UC3;
            conv = FlyCapture2::PIXEL_FORMAT_RGB8;
        return true;
        default:
            s_fm = 0;
            return false;
    }
}
