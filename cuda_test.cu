#include "cuda_test.h"
using namespace std;
using namespace cv;

char filename[] = "im2.png";
char dir[] = "/playpen/teddy";
char path[30];
int filter_size = 19;
int sigma = 3;
__global__
void blur_filter(uchar* orig, uchar* blur, int* fsize){
    int rows = gridDim.x;
    int cols = blockDim.x;
    int r = *fsize/2;
    int current_pos = blockIdx.x * cols + threadIdx.x;
    if((int)blockIdx.x - r < 0 || blockIdx.x + r >= rows || (int)threadIdx.x - r < 0 || threadIdx.x + r >= cols){
        blur[current_pos] = 0;
    }else{
        int total = 0;
        for(int i = -r; i < r+1; i++){
            for(int j = -r; j < r+1; j++){
                total += (int)orig[current_pos + i*cols + j];
            }
        }
        blur[current_pos] = (uchar)(total/(float)(*fsize * *fsize));
    }
}

__global__
void gaussian_filter(uchar* orig, uchar* g_res, int* sigma, int* fsize){
    int rows = gridDim.x;
    int cols = blockDim.x;
    int r = *fsize/2;
    int cpos = blockIdx.x * cols + threadIdx.x;
    if((int)blockIdx.x - r < 0 || blockIdx.x + r >= rows || (int)threadIdx.x - r < 0 || threadIdx.x + r >= cols){
        g_res[cpos] = 0;
    }else{
        int g_val = 0;
        for(int i = -r; i < r+1 ; i++){
            for(int j = -r; j < r+1; j++){
                float gc = (1/(2*3.1415926*(*sigma)*(*sigma)))*expf(-1*(i*i+j*j)/((float)2*(*sigma)*(*sigma)));
                g_val += (gc * orig[cpos + i*cols + j]);
            }
        }
        g_res[cpos] = (uchar)g_val;
    }
}

__global__
void edge_detector(uchar* orig, uchar* edge){
    int rows = gridDim.x;
    int cols = blockDim.x;
    int cpos = blockIdx.x * cols + threadIdx.x;
    if(blockIdx.x == 0 || threadIdx.x == 0 || blockIdx.x ==rows-1 || threadIdx.x == cols-1){
        edge[cpos] = 0;
    }else{
        //apply sobel filter
        int totalx = 0, totaly = 0;
        int frows = -1,fcols = -1;
        for(frows = -1; frows < 2; frows++){
            for(fcols = -1; fcols < 2; fcols++){
                int xsign = (fcols == -1)?1:(fcols == 0)?0:-1;
                int xc = (frows == 0)?2:1;
                int ysign = (frows == -1)?1:(frows == 0)?0:-1;
                int yc = (fcols == 0)?2:1;

                totalx += (int)orig[cpos + frows * cols + fcols] * (xc * xsign);
                totaly += (int)orig[cpos + frows * cols + fcols] * (yc * ysign);
            }
        }
        edge[cpos] = (uchar)sqrtf(totalx * totalx + totaly * totaly);
    }
}

//void add_v(int *a, int *b, int *c){
//    //*c = *a + *b;
//    c[threadIdx.x] = a[threadIdx.x] + b[threadIdx.x];
//}

extern __device__ int shared_img[256];

void random_ints(int *a, int range){
    int i;
    for(i = 0; i < range; i++){
        a[i] = rand()%50;
    }
}

void print_vector(int* a){
    int i  = 0;
    for(i = 0; i < N; i++){
        if(i == N -1){
            printf("%d\n", a[i]);
            continue;
        }
        printf("%d ",a[i]);
    }
}

//int main(void){
////    int *a,*b;
////    int *c;
////    int *d_a, *d_b, *d_c;
////    int size = N * sizeof(int);

////    cudaMalloc((void **) &d_a, size);
////    cudaMalloc((void **) &d_b, size);
////    cudaMalloc((void **) &d_c, size);
//    uchar* d_orig;
//    uchar* d_res, *res, * d_gres;
//    int* d_fsize;
//    int* d_sigma;

//    sprintf(path, "%s/%s", dir, filename);
//    Mat img = imread(path, CV_LOAD_IMAGE_GRAYSCALE);
//    if(!img.data){
//        return 1;
//    }
//    int rows = img.rows;
//    int cols = img.cols;
//    int img_dim = cols * rows;
//    printf("size is %d\n", img_dim);

//    int size = img_dim * sizeof(uchar);
//    //initialization
//    cudaMalloc((void **) &d_sigma, sizeof(int));
//    cudaMalloc((void **) &d_fsize, sizeof(int));
//    cudaMalloc((void **) &d_orig, size);
//    cudaMalloc((void **) &d_res, size);
//    cudaMalloc((void **) &d_gres, size);
//    res = (uchar*)malloc(size);

//    DFS_CUDA_ASSERT(cudaMemcpy(d_fsize, &filter_size, sizeof(int), cudaMemcpyHostToDevice));
//    DFS_CUDA_ASSERT(cudaMemcpy(d_sigma, &sigma,sizeof(int),cudaMemcpyHostToDevice));
//    DFS_CUDA_ASSERT(cudaMemcpy(d_orig, img.data, size, cudaMemcpyHostToDevice));
//    //blur_filter<<<rows, cols>>>(d_orig, d_res, d_fsize);

//    gaussian_filter<<<rows,cols>>>(d_orig,d_gres,d_sigma,d_fsize);
//    edge_detector<<<rows, cols>>>(d_gres,d_res);

//    DFS_CUDA_ASSERT(cudaPeekAtLastError());
//    DFS_CUDA_ASSERT(cudaMemcpy(res, d_res, size, cudaMemcpyDeviceToHost));

//    DFS_CUDA_ASSERT(cudaFree(d_fsize));
//    DFS_CUDA_ASSERT(cudaFree(d_sigma));
//    DFS_CUDA_ASSERT(cudaFree(d_gres));
//    DFS_CUDA_ASSERT(cudaFree(d_orig));
//    DFS_CUDA_ASSERT(cudaFree(d_res));


//    Mat output(rows,cols,CV_8UC1,res);
//    imwrite("Guassian3&Edges.png", output);
//    namedWindow("Original", WINDOW_AUTOSIZE );
//    imshow("Original",img);
//    namedWindow("Gaussian", WINDOW_AUTOSIZE );
//    imshow("Gaussian",output);
//    waitKey(0);
//    free(res);



////    a = (int *)malloc(size);
////    b = (int *)malloc(size);
////    random_ints(a,N);
////    random_ints(b,N);
////    c = (int *)malloc(size);

////    DFS_CUDA_ASSERT(cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice));
////    DFS_CUDA_ASSERT(cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice));
////    add_v<<<1,N>>>(d_a, d_b, d_c);
////    DFS_CUDA_ASSERT(cudaPeekAtLastError());
////    DFS_CUDA_ASSERT(cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost));

////    print_vector(a);
////    print_vector(b);
////    print_vector(c);

////    free(a);
////    free(b);
////    free(c);
////    DFS_CUDA_ASSERT(cudaFree(d_a));
////    DFS_CUDA_ASSERT(cudaFree(d_b));
////    DFS_CUDA_ASSERT(cudaFree(d_c));

//    return 0;
//}
