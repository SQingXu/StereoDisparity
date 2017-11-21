#include "flycaptureplayer.h"

class DFSVPlayer{
public:
    DFSVPlayer(String in);
    ~DFSVPlayer();
    void Open();
    void Close();
    void GrabNextFrame(vector<StreamPacket>& sp);

    int GetFrameNumber(){return num_frames;}
    int GetCurrentFrameNumber(){return current_frame;}
    void SetCurrentFrameNumber(unsigned int frame){current_frame = frame%num_frames;}
    void SelectFrame(vector<StreamPacket>& sp, unsigned int frame);

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
