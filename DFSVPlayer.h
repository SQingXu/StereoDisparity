#pragma once
#include "StreamUtil.h"

class DFSVPlayer{
public:
    DFSVPlayer(String in);
    ~DFSVPlayer();
    void Open();
    void Close();
    void GrabNextFrame(vector<StreamPacket>& sp);

    unsigned int GetFrameNumber(){return num_frames;}
    unsigned int GetCurrentFrameNumber(){return current_frame;}
    void SetCurrentFrameNumber(unsigned int frame){current_frame = frame%num_frames;}
    unsigned int GetNumStreams(){return num_streams;}
    StreamInfo GetStreamInfo(){return streamInfos[0];}
 private:
    size_t size_bytes;
    size_t frame_bytes;
    size_t header_bytes;
    size_t image_buffer_bytes;
    String in_path;
    ifstream in_file;
    unsigned int num_streams;
    unsigned int num_frames;
    unsigned int current_frame;
    vector<StreamPacket> streamPackets;
    vector<StreamInfo> streamInfos;
};
