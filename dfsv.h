#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <fstream>

enum SImageFormat
{
	S_GRAY8,
	S_GRAY16,
	S_GRAY32,
	S_RGB24,
	S_RGB96,
	S_RGBA32,
	S_RGBA128,
	S_NONE
};
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
	SImageFormat fmt;
};

struct StreamPacket
{

	cv::Mat image_buffer;

	TimeStamp tStamp;

	SImageFormat fmt;

	size_t size;

	void initialize(const int width, const int height, SImageFormat fmt)
	{
		this->fmt = fmt;
		
		image_buffer = cv::Mat::zeros(cv::Size(width, height), getCVFmt(fmt));
		size = width * height * GetBpp(fmt) / 8;
	}

	size_t SizeBytes() { return size; }

	size_t getCVFmt(SImageFormat fmt)
	{
		switch (fmt)
		{
		case S_GRAY8:
			return CV_8UC1;
		case S_GRAY16:
			return CV_16UC1;
		case S_GRAY32:
			return CV_32FC1;
		case S_RGB24:
			return CV_8UC3;
		case S_RGB96:
			return CV_32FC3;
		case S_RGBA32:
			return CV_8UC4;
		case S_RGBA128:
			return CV_32FC4;
		case S_NONE:
			return 0;
		default:
			return 0;
		}
	}

	size_t GetBpp(const SImageFormat fmt)
	{
		switch (fmt)
		{
		case S_GRAY8:
			return 8;
		case S_GRAY16:
			return 16;
		case S_GRAY32:
			return 32;
		case S_RGB24:
			return 24;
		case S_RGB96:
			return 96;
		case S_RGBA32:
			return 32;
		case S_RGBA128:
			return 128;
		case S_NONE:
			return 0;
		default:
			return 0;
		}
	}

	void CopyFrom(const StreamPacket& streamPacket)
	{
		this->fmt = streamPacket.fmt;
		this->size = streamPacket.size;
		this->image_buffer = streamPacket.image_buffer.clone();
        if(this->fmt == S_RGB24){
            cv::cvtColor(image_buffer, image_buffer, CV_RGB2BGR);
        }
		this->tStamp = streamPacket.tStamp;
	}
};

class DFSVReader
{
	public:
		DFSVReader(const std::string& input) :input_url(input) {}

		~DFSVReader();

		void Open();

		// Size of image_buffer only
		size_t SizeBytes() const { return size_bytes; }

		unsigned int GetNumStreams() const { return num_streams; }

		void Start();

		void Stop();

		void GrabNextFrame(std::vector<StreamPacket>& streamPackets);

		void SetStreamProperties(const std::vector<StreamInfo>& streamPros) { streamInfos.resize(streamPros.size()); streamInfos = streamPros; }

		const std::vector<StreamInfo>& GetStreamProperties() const { return streamInfos; }

		unsigned int GetNumFrames() const { return num_frames; }

		unsigned int GetCurrentFrameId() const { return current_frame; }

		void SetCurrentFrameId(unsigned int frameId)
		{
			if (frameId < num_frames)
				current_frame = frameId;
		}

	private:
		std::string input_url;
		std::vector<StreamPacket> streamPackets;
		std::vector<StreamInfo> streamInfos;
		std::ifstream in_file;
		unsigned int num_streams;
		unsigned int num_frames;
		unsigned int current_frame;
		size_t size_bytes;
		size_t header_bytes;
		size_t frame_bytes;
};
