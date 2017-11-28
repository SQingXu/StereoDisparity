#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include "include/Eigen/Dense"
#include "include/Eigen/Sparse"

using namespace Eigen;
using namespace cv;
using namespace std;

class Regularizer{
public:
    Regularizer(int s);
    ~Regularizer();
    void Regularize(Mat raw_depth, Mat depth);
    void LoadAlphas(int pos, float alpha);
private:
    int size;
    vector<float> A_v;
    SparseMatrix<float> A;
    vector<float> B;
    void LoadCoefficients(Mat raw_depth);

};
