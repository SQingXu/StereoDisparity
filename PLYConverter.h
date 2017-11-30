#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include "camera.h"

using namespace std;
using namespace cv;

class PLYConverter{
public:
    PLYConverter();
    ~PLYConverter();
    void writeDepthToPLY(const char* path, Mat depth,Mat color, Camera c, bool isColor);
};
