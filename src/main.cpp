#include <iostream>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

double focus(cv::Mat &src) {
    cv::Mat src_gray, dst;
    int kernel_size = 3;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;

    GaussianBlur( src, src, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );
    cvtColor( src, src_gray, cv::COLOR_RGB2GRAY );
    // cv::Mat abs_dst;
    // Laplacian( src_gray, dst, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT );
    cv::Laplacian(src_gray, dst, CV_64F);
    cv::Scalar mu, sigma;
    cv::meanStdDev(dst, mu, sigma);
    double focusMeasure = sigma.val[0] * sigma.val[0];
    return focusMeasure;
}

int main(int argc, char **argv) {
    std::cout << "Hello, World!" << std::endl;

    printf("these are the %d arguments\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d] = \"%s\"", i, argv[i]);
    }

    cv::VideoCapture cap(2);
    if (!cap.isOpened()) { return -1; }

    cv::Mat edges;
    cv::Mat image;
    cv::namedWindow("edges", 1);

    for(;;)
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
//        GaussianBlur(edges, edges, cv::Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
//        imshow("edges", edges);
        cv::imshow("edges", edges);
        cv::imshow("frame", frame);

        double focus_measure = focus(frame);
        printf("focus = %f\n", focus_measure);

        if(cv::waitKey(200) >= 0) break;
    }

    return 0;
}
