#include <iostream>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

int main(int argc, char **argv) {
    std::cout << "Hello, World!" << std::endl;

    printf("these are the %d arguments\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d] = \"%s\"", i, argv[i]);
    }

    cv::VideoCapture cap(2);
    if (!cap.isOpened()) { return -1; }

    cv::Mat edges;
    cv::namedWindow("edges", 1);

    for(;;)
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
        GaussianBlur(edges, edges, cv::Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if(cv::waitKey(200) >= 0) break;
    }

    return 0;
}
