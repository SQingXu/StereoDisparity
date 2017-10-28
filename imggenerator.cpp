#include "dfsv.h"
#include <sys/stat.h>

char* cam_str[] = { "PointGrey1","PointGrey2","gopro4","picamera1","picamera2" };
char* out_fmt = "jpg";

int main()
{
	char rootDir[256], inputDir[256], outputDir[256],inputFile[256];

    sprintf(rootDir, "/playpen/StereRecording/2/");
    sprintf(inputDir, "%s", rootDir);
    sprintf(outputDir, "%s/output", rootDir);

    sprintf(inputFile, "%s/record1.dfsv", inputDir);

	DFSVReader dfsvReader(inputFile);
	dfsvReader.Open();
	dfsvReader.Start();
	size_t num_frames = dfsvReader.GetNumFrames();
	size_t num_streams = dfsvReader.GetNumStreams();
	std::vector<FILE*> timestamps;
	timestamps.resize(num_streams);

	for (size_t s = 0; s < num_streams; s++)
	{
		char outDir[256];
		sprintf(outDir, "%s/%s", outputDir,cam_str[s]);
        mkdir(outDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		char tStamp[256];
		sprintf(tStamp, "%s/tStamp.txt", outDir);
		timestamps[s] = fopen(tStamp, "w+");
	}

	std::vector<StreamPacket> streams(num_streams);
	
	

	for (size_t i = 0; i < num_frames; ++i)
	{
		dfsvReader.SetCurrentFrameId(i);
		dfsvReader.GrabNextFrame(streams);

		for (size_t s = 0; s < num_streams; s++)
		{
			char outFile[256];
			sprintf(outFile, "%s/%s/%05d.%s", outputDir, cam_str[s],i,out_fmt);
			cv::imwrite(outFile, streams[s].image_buffer);
			fprintf(timestamps[s], "FrameId: %d\n", i);
			fprintf(timestamps[s], "Time: %lf\n", streams[s].tStamp.seconds);
			cv::Mat pose = cv::Mat::zeros(4,4,CV_32FC1);
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					fprintf(timestamps[s], "%f ", pose.at<float>(j,k));
				}
				fprintf(timestamps[s], "\n");
			}
		}
	}
	for (size_t s = 0; s < num_streams; s++)
		fclose(timestamps[s]);

	return 0;
}
