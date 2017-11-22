#pragma once
#include <iostream>
#include <stdlib.h>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

class Calibrator{
    Calibrator();
    ~Calibrator();

    void IntrisicCalibration();

};
