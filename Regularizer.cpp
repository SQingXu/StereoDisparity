#include "Regularizer.h"

Regularizer::Regularizer(int s){
    //size means number of pixels in the image
    size = s;
    A_v.resize(size * size, 0);
}

Regularizer::~Regularizer(){

}

Regularizer::LoadAlphas(int pos, float alpha){
    A_v[pos * size + pos] += alpha;
}

Regularizer::LoadCoefficients(Mat raw_depth){

}
