#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include "include/Eigen/Dense"
#include "include/Eigen/Sparse"
#include "include/Eigen/SparseCholesky"
#include "include/Eigen/IterativeLinearSolvers"

using namespace Eigen;
using namespace cv;
using namespace std;

class Regularizer{
public:
    Regularizer();
    ~Regularizer();
    void Start(int s);
    void Regularize(Mat raw_depth, Mat& depth, float beta, int Iteration);
    void LoadAlphas(int pos, float alpha);
private:
    int size;
    vector<float> alphas;
    SparseMatrix<float, RowMajor> A;
    VectorXf B;
    void LoadCoefficients(Mat raw_depth, float beta);
};
