#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#include "frame-process.hpp"

ImageProcessStack::ImageProcessStack() {
    //    mat_head = 0;
    //    mat_tail = 0;
    //    mat_mask = 2 - 1;
    mat_back = 0; // &mat[0];
    mat_fore = 0; // &mat[1];
}

void ImageProcessStack::process_image(void *data, unsigned int size) {
    if (mat_back) { free(mat_back); }
    // mat_back = cv::Mat(1440, 1280, data);
}

ImageProcessStack *stack;

extern "C" {

    int image_process_stack_initialize(unsigned int rows, unsigned int cols)
    {
        stack = new ImageProcessStack;
        cv::namedWindow("the-window", 1);
        stack->rows = rows;
        stack->cols = cols;
        return 0;
    }

    void image_process_stack_process_image(void *data, unsigned int size)
    {
        unsigned int type = (size / (stack->rows * stack->cols) == 1) ? CV_8U : CV_16U;
        static uint8_t buff[640 * 480];
        uint8_t *p = (uint8_t *) data;
        unsigned int index = 0;
        for (int i = 0; i < 640; ++i) {
            for (int j = 0; j < 480; ++j) {
                buff[index++] = *p++;
                ++p;
            }
        }
        // cv::Mat mat(stack->rows, stack->cols, type, data);
        cv::Mat mat(stack->rows, stack->cols, CV_8U, buff);
        cv::imshow("the-window", mat);
        if(cv::waitKey(200) >= 0) { ; }
    }

}
