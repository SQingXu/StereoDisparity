#include "Regularizer.h"

Regularizer::Regularizer(){

}

Regularizer::~Regularizer(){

}

void Regularizer::Start(int s){
    //size means number of pixels in the image
    size = s;
    A.resize(size, size);
    alphas.resize(size, 0);
    B.resize(s);
}

void Regularizer::Regularize(Mat raw_depth, Mat& depth, float beta, int Iteration){
    int rows = raw_depth.rows;
    int cols = raw_depth.cols;
    depth = Mat(rows, cols, CV_32FC1);

    LoadCoefficients(raw_depth, beta);
    VectorXf x(rows * cols);
    VectorXf guess(rows * cols);
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            guess[r * cols + c] = raw_depth.at<float>(r,c);
        }
    }

    //**** RowMajor Here is important *****
    ConjugateGradient<SparseMatrix<float,RowMajor>, Lower|Upper> cg;
    cg.setMaxIterations(15);
    cg.compute(A);
    x = cg.solveWithGuess(B,guess);

//    std::cout << "#iterations: " << cg.iterations() << std::endl;
//    std::cout << "estimated error: " << cg.error() << std::endl;

    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            int index = cols * r + c;
            depth.at<float>(r,c) = x[index];
        }
    }

}

/* Loading the alpha according to the passing alpha
 * If the value of alpha is less than 0 or somehow
 * meaningless (-nan) then just set as 0
 */
void Regularizer::LoadAlphas(int pos, float alpha){
    alphas[pos] = (alpha < 0 || isnan(alpha))? 0:alpha;
}

void Regularizer::LoadCoefficients(Mat raw_depth, float beta){
    //when load coefficients is called, we can suppose alphas finished loading
    float neighbor_diff_threshold = 4.0;

    A.setZero();
    int rows = raw_depth.rows;
    int cols = raw_depth.cols;
    //ignore the edge
    for(int r = 1; r < rows-1; r++){
        for(int c = 1; c < cols-1; c++){
            int index = cols * r + c;
            int val_o = raw_depth.at<float>(r,c);

            //if depth value of current pixel is 0 then set b to 0 and ignore
            if(val_o == 0){
                B[index] = 0;
                continue;
            }
            A.coeffRef(index,index) = 2 * alphas[index];

            /* if depth value of neighbor pixels is 0 or
             * the absolute different is larger than threshold,
             * then set weight to 0 (dismiss the loop)
             */
            for(int i = 0; i < 4; i++){
                int up = (i==0)?1:0;
                int dw = (i==1)?1:0;
                int lf = (i==2)?1:0;
                int rt = (i==3)?1:0;
                int new_r = r + up - dw;
                int new_c = c + rt - lf;

                float val_n = raw_depth.at<float>(new_r,new_c);
                float f_abs = fabs(val_n - val_o);
                if(val_n == 0 || f_abs > neighbor_diff_threshold){
                    continue;
                }

                //add to diagonal
                float weight = 1.0/(1 + beta * f_abs);
                A.coeffRef(index,index) += 2 * 2 * weight;

                /* set coefficient of sparse matrix of same row with
                 * column number correponds to the position of neighboring
                 * pixels
                 */
                int new_index = new_r * cols + new_c;
                A.coeffRef(index, new_index) = -2 * 2 * weight;
            }

            B[index] = 2 * val_o * alphas[index];
        }
    }
}


