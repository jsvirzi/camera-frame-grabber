#ifndef FRAME_PROCESS_H
#define FRAME_PROCESS_H

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

class ImageProcessStack {
public:
//    cv::Mat mat[2];
//    unsigned int mat_head;
//    unsigned int mat_tail;
//    unsigned int mat_mask;
    cv::Mat *mat_back;
    cv::Mat *mat_fore;
    ImageProcessStack();
    void process_image(void *data, unsigned int size);
    unsigned int rows;
    unsigned int cols;
    unsigned int pixel_format;
    const char *window_name;
    uint8_t *buff;
    ssize_t buff_size;
};

#endif
