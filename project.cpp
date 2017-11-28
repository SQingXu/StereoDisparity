#include "redepth.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>


char filename1[] = "/playpen/StereRecording/2/CalibResult3/calib_cam1.txt";
char filename2[] = "/playpen/StereRecording/2/CalibResult3/calib_cam2.txt";

char img1[] = "/playpen/StereRecording/2/output/PointGrey1/00211.jpg";
char img2[] = "/playpen/StereRecording/2/output/PointGrey2/00211.jpg";
int main(void)
{

////    Point2f points(400.5, 200.2);
//    float data1[2] = {400.5, 200.2};
//    Mat points(2,1,CV_32FC1,&data1);
//    Mat unpoints(3,1,CV_32FC1);
//    c2.unprojectPt(points,unpoints,50.0);
////    c1.undistortPt(points,unpoints);
//    printf("result: %10f %10f %10f\n",unpoints.at<float>(0,0), unpoints.at<float>(1,0), unpoints.at<float>(2,0));
//    c1.projectPt(unpoints,points);
//    printf("result: %10f %10f\n",points.at<float>(0,0), points.at<float>(1,0));
    printf("Test\n");

//    if(!glfwInit()){
//        fprintf(stderr, "Failed to initialize GLFW\n");
//        return -1;
//    }

//    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

//    // Open a window and create its OpenGL context
//    GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
//    window = glfwCreateWindow( 1024, 768, "DepthMap", NULL, NULL);
//    if( window == NULL ){
//        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
//        glfwTerminate();
//        return -1;
//    }

//    glfwMakeContextCurrent(window); // Initialize GLEW
//    glewExperimental=true; // Needed in core profile
//    if (glewInit() != GLEW_OK) {
//        fprintf(stderr, "Failed to initialize GLEW\n");
//        return -1;
//    }

//    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

//    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

//    //VAO Object creation and binding
//    GLuint VertexArrayID;
//    glGenVertexArrays(1,&VertexArrayID);
//    glBindVertexArray(VertexArrayID);

//    static const GLfloat g_vertex_buffer_data[] = {
//      -1.0f, -1.0f, 0.0f,
//       1.0f, -1.0f, 0.0f,
//       0.0f,  1.0f, 0.0f,
//    };

//    GLuint vertexbuffer;
//    glGenBuffers(1,&vertexbuffer);
//    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

//    do{
//        glEnableVertexAttribArray(0);
//        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//        glVertexAttribPointer(
//                    0,
//                    3,
//                    GL_FLOAT,
//                    GL_FALSE,
//                    0,
//                    (void*)0
//        );
//        glDrawArrays(GL_TRIANGLES,0,3);
//        glDisableVertexAttribArray(0);

//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//    while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window)==0);


    ReverseDepth rd = ReverseDepth();
    rd.SetRange(50,100);
    rd.ReadImagePair(img1, img2, filename1, filename2);
    Mat depth = rd.FindDepth(true, true, ZNCC);
    imwrite("zncc_output.jpg",depth);
    namedWindow("Test Window", WINDOW_AUTOSIZE );
    imshow("Test Window",depth);
    waitKey(0);
    return 0;
}
