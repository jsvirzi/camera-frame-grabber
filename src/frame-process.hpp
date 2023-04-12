#ifndef FRAME_PROCESS_H
#define FRAME_PROCESS_H

#include <pthread.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

typedef struct {
    int roi_select_state;
    struct { int x, y; } pt1, pt2;
} FrameWindowParameters;

class ImageProcessStack {
public:
    cv::Mat *mat_back;
    cv::Mat *mat_fore;
    ImageProcessStack();
    void process_image(void *data, unsigned int size);
    unsigned int rows;
    unsigned int cols;
    unsigned int pixel_format;
    const char *window_name;
    uint8_t **buff;
    ssize_t buff_size;
    uint8_t **roi_buff;
    unsigned int roi_stride_x;
    unsigned int roi_stride_y;
    unsigned int roi_buff_size;
    unsigned int buff_head;
    unsigned int buff_tail;
    unsigned int buff_mask;
    unsigned int run;
    unsigned int n_roi_x;
    unsigned int n_roi_y;
    // unsigned int roi_x[N_ROI];
    // unsigned int roi_y[N_ROI];
    unsigned int *roi_buff_index;
    uint8_t *roi_index;
    pthread_t thread_id;
    double *focus_measure;
    double focus_measure_img;
    double focus_measure_roi;
    FrameWindowParameters frame_window_params;
};

#endif
