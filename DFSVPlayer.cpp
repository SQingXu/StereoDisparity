#include "DFSVPlayer.h"

DFSVPlayer::DFSVPlayer(String in){
    in_path = in;
}

DFSVPlayer::~DFSVPlayer(){

}

/* This function should be called first; It reads in the basic
 * information about the video file: length of file in bytes,
 * total number of frames, number of stream and it resize the
 * vectors which store the StreamPacket and StreamInfo corresponding
 * with the number of stream; Then it reads in streamInfo struct
 * of the video file which specify video's each frame's width,
 * height and Mat CV format
 */
void DFSVPlayer::Open(){
    in_file.open(in_path.c_str(), ifstream::binary);
    // get length of file
    in_file.seekg(0,in_file.end);
    size_bytes = in_file.tellg();

    // get number of frames
    in_file.seekg(0);
    in_file.seekg(size_bytes - sizeof(num_frames), in_file.beg);
    in_file.read((char *)&num_frames, sizeof(num_frames));

    // get number of streams
    in_file.seekg(0);
    in_file.read((char *)&num_streams, sizeof(num_streams));

    streamPackets.resize(num_streams);
    streamInfos.resize(num_streams);

    std::cout << "***DFSV Video Information***" << std::endl
        << "Video Size bytes " << size_bytes << std::endl
        << "Num Streams " << num_streams << std::endl
        << "Num Frames " << num_frames << std::endl
        << std::endl;

    frame_bytes = 0;

    for(unsigned int i = 0; i < num_streams; i++){
        in_file.read((char *)&streamInfos[i], sizeof(StreamInfo));
        streamPackets[i].image_buffer = Mat(streamInfos[i].h, streamInfos[i].w, streamInfos[i].cvfmt);

        std::cout  << "***Stream: " << i << " ***" << std::endl
                  << " Width " << streamInfos[i].w << std::endl
                  << " Height " << streamInfos[i].h << std::endl
                  << " Fmt " << streamInfos[i].cvfmt << std::endl
//                  << streamPackets[i].image_buffer. << std::endl
                  << std::endl;
        /* frame_bytes equal to the size of the image data and one size_t
         *  which indicates the time stamp of the frame */
        image_buffer_bytes = streamPackets[i].image_buffer.step[0] * streamPackets[i].image_buffer.rows;
        frame_bytes += (image_buffer_bytes + sizeof(size_t));

    }

    //reset the current frame number and read in the size of header part of the file
    current_frame = 0;
    header_bytes = in_file.tellg();
    std::cout << header_bytes << std::endl;

}

void DFSVPlayer::GrabNextFrame(vector<StreamPacket> &sp){
    //put the file read pointer to the start of current frame
    in_file.seekg(0);
    in_file.seekg(header_bytes + (current_frame * frame_bytes), in_file.beg);

    for(unsigned int i = 0; i < num_streams; i++){
        in_file.read((char *)streamPackets[i].image_buffer.data, image_buffer_bytes);
        in_file.read((char *)&streamPackets[i].tStamp.microSeconds, sizeof(size_t));
        streamPackets[i].tStamp.seconds = (double)streamPackets[i].tStamp.microSeconds * 1e-6;
        sp[i].CopyFrom(streamPackets[i]);
    }
}

void DFSVPlayer::Close(){
    in_file.close();
}

void DFSVPlayer::SelectFrame(vector<StreamPacket> &sp, unsigned int frame){
    int selected_frame = frame%num_frames;

    //put the file read pointer to the start of current frame
    in_file.seekg(0);
    in_file.seekg(header_bytes + (selected_frame * frame_bytes), in_file.beg);

    for(unsigned int i = 0; i < num_streams; i++){
        in_file.read((char *)&streamPackets[i].image_buffer.data, image_buffer_bytes);
        in_file.read((char *)&streamPackets[i].tStamp.microSeconds, sizeof(size_t));
        streamPackets[i].tStamp.seconds = (double)streamPackets[i].tStamp.microSeconds * 1e-6;
        sp[i].CopyFrom(streamPackets[i]);
    }
}
