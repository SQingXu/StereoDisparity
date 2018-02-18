#include "DFSVPlayer.h"
#include "FlyCapturePlayer.h"
#include "Calibrator.h"

#include <ctime>
#include <chrono>
#include <unistd.h>

#define PLAY "play"
#define RECORD "record"
#define PLAY_WIN "Play Video"
#define RECORD_WIN "Stereo Stream"
#define SELECT_WIN "Select Frames"
#define FRAME_LIMIT 200;

enum AppMode{
    Record,
    PlayDFSV
};

void ShowPairImages(vector<StreamPacket> &sp, const String win_name, size_t cv_fmt);

String video_path = "/playpen/StereoDisparity/Capture/calib_2-18";

/* A simple interface for either render real-time or recorded video
 * In the Record Mode:
 *   r: Start Recording
 *   s: Stop Recording
 * In the Play DFSV Mode
 *   p: Pause/Resume the video
 *   s: select frames for calibration
 *   d: un-select current frame if current frame is selected
 *   g: Go to specifeid frame number
 *   f: Change Frame Rate to input value
 *   t: Change Step value to input value
 *   i: Move Backward by one step if not image for calibration, by one
 *      one frame if image for calibration
 *   o: Move Forward by one step if not image for calibration, by one
 *      one frame if image for calibration
 *   */
int main(int argc, char *argv[]){
    //String path = "/playpen/StereoDisparity/Capture/capture2_calib";
    char keypress;
    AppMode am;

    if(argc != 2){
        std::cout << "invalid number of arguments, expected exactly 1" << std::endl;
        return 0;
    }
    const char* arg1 = argv[1];
    if(strcmp(PLAY, arg1) == 0){
        //Play Video File
        am = PlayDFSV;
    }else if(strcmp(RECORD, arg1) == 0){
        //Record & Rendering Real-Time
        am = Record;
    }else{
        std::cout << "Invalid Argument" << std::endl;
        return 0;
    }
    if(am == Record){
        std::cout << "Real Time Rendering" << std::endl;
        bool record = false;
        FlyCapturePlayer player;
        player.Open();
        player.Start();
        cv::namedWindow(RECORD_WIN, WINDOW_AUTOSIZE);
        do{
            std::vector<StreamPacket> packets;
            packets.resize(2);
            player.GrabNextFrame(packets);
            if(record){
                player.WriteStreams(packets);
            }
            ShowPairImages(packets, RECORD_WIN,CV_16UC1);

            keypress = (cv::waitKey(1) & 0xff);

            if(keypress == 'r'){
                std::cout << "sense the keypressed r" << std::endl;
                if(!record){
                    record = true;
                    String out_path = video_path;
                    //sprintf(&out_path,"%s/capture%d",out_dir, 1);
                    player.StartRecord(out_path);
                }
            }

            if(keypress == 's'){
                std::cout << "sense the keypressed s" << std::endl;
                if(record){
                    record = false;
                    player.StopRecord();
                }
            }
        }while(keypress != 'q');
        player.Stop();
    }else{
        bool pause = false;
        bool calibration = false;
        vector<unsigned int> selected_frames;
        float frame_rate = 30.0;
        std::cout << "Play Video" << std::endl;

        DFSVPlayer dplayer;
        Calibrator calibrator;
        dplayer.Open(video_path);
        unsigned int current_frame = 0;
        unsigned int step = 1;
        unsigned int calib_index = 0;
        unsigned int prev_frame = 0;
        unsigned int frame_number = dplayer.GetFrameNumber();
        std::vector<StreamPacket> packets;
        packets.resize(2);

        cv::namedWindow(SELECT_WIN ,WINDOW_AUTOSIZE);
        cv::namedWindow(PLAY_WIN ,WINDOW_AUTOSIZE);
        do{
            if(!pause && !calibration){
                //get time stamp at start of loop
                chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

                dplayer.GrabNextFrame(packets);
                ShowPairImages(packets, PLAY_WIN,dplayer.GetStreamInfo().cvfmt);

                //update current frame number
                current_frame = dplayer.GetCurrentFrameNumber();
                current_frame = (current_frame + 1)%frame_number;
                dplayer.SetCurrentFrameNumber(current_frame);

                //get time stamp at end of loop
                chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

                //sleep the program to match the frame rate
                chrono::duration<double> time_span = t2 - t1;
                size_t ms = (size_t)(time_span.count() * 1e6);
                size_t target_span = (size_t)((float)1e6/frame_rate);
                if(target_span > ms){
                    usleep(target_span - ms);
                }
            }

            //update keypress
            keypress = (cv::waitKey(1) & 0xff);
            if(keypress == 'p'){
                // pause & resume
                if(!pause){
                    pause = true;
                }else{
                    pause = false;
                }
            }

            // go to specific frame
            if(keypress == 'g'){
                unsigned int i;
                std::cout << "Enter the specific frame number: ";
                std::cin >> i;
                std::cout << std::endl;
                if(i >= frame_number){
                    //reset the frame number to zero in this case
                    i = 0;
                }
                dplayer.SetCurrentFrameNumber(i);
                dplayer.GrabNextFrame(packets);
                ShowPairImages(packets, PLAY_WIN,dplayer.GetStreamInfo().cvfmt);
            }

            //change the frame rate
            if(keypress == 'f' && !calibration){
                unsigned int i;
                std::cout << "Enter the expected frame rate: ";
                std::cin >> i;
                std::cout << std::endl;
                if(i <= 200){
                    frame_rate = (float)i;
                }
                std::cout << "Current Frame Rate: " << frame_rate << std::endl;
            }

            //change step
            if(keypress == 't'){
                unsigned int i;
                std::cout << "Expected step: ";
                std::cin >> i;
                std::cout << std::endl;
                if(i <= frame_number){
                    step = i;
                }
            }


            //select frame
            if(keypress == 's' && !calibration){
                bool repeated = false;
                unsigned int frame = dplayer.GetCurrentFrameNumber();
                for(int i = 0; i < selected_frames.size(); i++){
                    if(selected_frames[i] == frame){
                        repeated = true;
                        break;
                    }
                }
                if(!repeated){
                    selected_frames.push_back(dplayer.GetCurrentFrameNumber());
                    std::cout << "One Frame selected, " << selected_frames.size() << "in total" << std::endl;
                }
            }

            //un-select frame
            if(keypress == 'd'){
                unsigned int frame = dplayer.GetCurrentFrameNumber();
                //find the frame number
                unsigned int i = 0;
                for(i = 0; i < selected_frames.size(); i++){
                    if(selected_frames[i] == frame){
                        selected_frames.erase(selected_frames.begin() + i);
                        std::cout << "One Frame removed, " << selected_frames.size() << "in total" << std::endl;
                        break;
                    }
                }
                if(selected_frames.size() == 0){
                    calibration = false;
                }
                if(calibration){
                    dplayer.SetCurrentFrameNumber(selected_frames[i]);
                    dplayer.GrabNextFrame(packets);
                    ShowPairImages(packets,SELECT_WIN,dplayer.GetStreamInfo().cvfmt);
                }

            }

            //View Selected Frames only valid when there is at least one frame selected
            if(keypress == 'c' ){
                if(!calibration){
                    if(selected_frames.size() != 0){
                        calibration = true;
                        calib_index = 0;

                        prev_frame = dplayer.GetCurrentFrameNumber();
                        dplayer.SetCurrentFrameNumber(selected_frames[calib_index]);
                        dplayer.GrabNextFrame(packets);
                        ShowPairImages(packets,SELECT_WIN,dplayer.GetStreamInfo().cvfmt);
                    }
                }else{
                    calibration = false;
                    dplayer.SetCurrentFrameNumber(prev_frame);
                    dplayer.GrabNextFrame(packets);
                    ShowPairImages(packets,PLAY_WIN,dplayer.GetStreamInfo().cvfmt);
                }
            }
            //move forward in frame
            if(keypress == 'o'){
                if(calibration){
                    calib_index = (calib_index + 1)%selected_frames.size();
                    dplayer.SetCurrentFrameNumber(selected_frames[calib_index]);
                    dplayer.GrabNextFrame(packets);
                    ShowPairImages(packets,SELECT_WIN,dplayer.GetStreamInfo().cvfmt);
                }else{
                    if(pause){
                        unsigned int current_frame = dplayer.GetCurrentFrameNumber();
                        dplayer.SetCurrentFrameNumber((current_frame + step)%frame_number);
                        dplayer.GrabNextFrame(packets);
                        ShowPairImages(packets,PLAY_WIN,dplayer.GetStreamInfo().cvfmt);
                    }
                }
            }
            //move backward in frame
            if(keypress == 'i'){
                if(calibration){
                    calib_index =(calib_index == 0)?(selected_frames.size()-1):(calib_index - 1)%selected_frames.size();
                    dplayer.SetCurrentFrameNumber(selected_frames[calib_index]);
                    dplayer.GrabNextFrame(packets);
                    ShowPairImages(packets,SELECT_WIN,dplayer.GetStreamInfo().cvfmt);
                }else{
                    if(pause){
                        unsigned int current_frame = dplayer.GetCurrentFrameNumber();
                        dplayer.SetCurrentFrameNumber((current_frame - step)%frame_number);
                        dplayer.GrabNextFrame(packets);
                        ShowPairImages(packets,PLAY_WIN,dplayer.GetStreamInfo().cvfmt);
                    }

                }
            }
            if(keypress == 'x'){
                if(calibration){
                    Size boardSize;
                    boardSize.width = 6;
                    boardSize.height = 4;
                    calibrator.LoadCalibInfo(boardSize,2.275,video_path,selected_frames);
                    calibrator.IntrisicCalibration();
                    calibrator.StereoExtrinsicCalibration();
                    calibrator.SaveCalibration();
                }
            }


        }while(keypress != 'q');
        dplayer.Close();
    }
    return 0;
}
void ShowPairImages(vector<StreamPacket>& sp, const String win_name, size_t cv_fmt){
    std::array<cv::Mat,2> raw_images = {
        sp[0].image_buffer,
        sp[1].image_buffer
    };
    // concatenate for display purposes
    cv::Mat image_pair(raw_images[0].rows, 2 * raw_images[0].cols, cv_fmt);
    cv::hconcat(raw_images[1], raw_images[0], image_pair);
    //std::cout << raw_images[0].rows << " " << raw_images[1].cols << std::endl;

    cv::imshow(win_name, image_pair);
}
