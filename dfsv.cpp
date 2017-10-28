#include "dfsv.h"

DFSVReader::~DFSVReader()
{
}

void DFSVReader::Open()
{
    in_file.open(input_url.c_str(), std::ifstream::binary);
	// get length of file:
	in_file.seekg(0, in_file.end);
	size_bytes = in_file.tellg();

	// get num frames
	in_file.seekg(0);
	in_file.seekg(size_bytes - sizeof(num_frames), in_file.beg);
	in_file.read((char*)&num_frames, sizeof(num_frames));
	in_file.seekg(0);


	in_file.read((char*)&num_streams, sizeof(num_streams));
	streamPackets.resize(num_streams);
	streamInfos.resize(num_streams);

}

void DFSVReader::Start()
{
	std::cout << "***DFSV Video Information***" << std::endl
		<< "Video Size bytes " << size_bytes << std::endl
		<< "Num Streams " << num_streams << std::endl
		<< "Num Frames " << num_frames << std::endl
		<< std::endl;
	frame_bytes = 0;
	for (unsigned int i = 0; i < num_streams; i++)
	{
		in_file.read((char*)&streamInfos[i], sizeof(StreamInfo));
		streamPackets[i].initialize(streamInfos[i].w, streamInfos[i].h, streamInfos[i].fmt);


		std::cout << "***Stream: " << i << " ***" << std::endl
			<< " Width " << streamInfos[i].w << std::endl
			<< " Height " << streamInfos[i].h << std::endl
			<< " Fmt " << streamInfos[i].fmt << std::endl
			<< streamPackets[i].fmt << std::endl
			<< std::endl;
		frame_bytes += streamPackets[i].SizeBytes() + sizeof(size_t);
	}
	current_frame = 0;
	header_bytes = in_file.tellg();
}

void DFSVReader::Stop()
{
	in_file.close();
}

void DFSVReader::GrabNextFrame(std::vector<StreamPacket>& streams)
{
	in_file.seekg(0);
	in_file.seekg(header_bytes + (current_frame* frame_bytes), in_file.beg);

	for (unsigned int i = 0; i < num_streams; i++)
	{
		in_file.read((char*)streamPackets[i].image_buffer.data, streamPackets[i].SizeBytes());

		in_file.read((char*)&streamPackets[i].tStamp.microSeconds, sizeof(size_t));

		streamPackets[i].tStamp.seconds = (double)streamPackets[i].tStamp.microSeconds * 1e-6;

		streams[i].CopyFrom(streamPackets[i]);
	}
}
