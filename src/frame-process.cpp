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

    int image_process_stack_initialize(unsigned int rows, unsigned int cols, unsigned int type)
    {
        stack = new ImageProcessStack;
        stack->window_name = "image";
        cv::namedWindow(stack->window_name, 1);
        stack->rows = rows;
        stack->cols = cols;
        stack->type = type;
        stack->buff_size = rows * cols * 4; /* TODO will this always suffice? */
        stack->buff = new uint8_t [stack->buff_size];
        return 0;
    }

    void image_process_stack_process_image(void const * const data, unsigned int size)
    {
        unsigned int type = (size / (stack->rows * stack->cols) == 1) ? CV_8U : CV_16U;
#if 1
        uint8_t *buff = stack->buff;
        uint8_t *p = (uint8_t *) data;
        unsigned int index = 0;
        for (int i = 0; i < stack->rows; ++i) {
            for (int j = 0; j < stack->cols; ++j) {
                buff[index++] = *p++;
                ++p;
            }
        }
        cv::Mat mat(stack->rows, stack->cols, CV_8U, buff);
#endif
        // cv::Mat mat(stack->rows, stack->cols, type, data);

#if 0
        uint8_t *buff = (uint8_t *) data;
        cv::Mat mat(stack->rows, stack->cols, CV_8U, buff);
#endif

        // cv::Mat mat(stack->rows, stack->cols, CV_8U, (uint8_t *) data);
        cv::imshow(stack->window_name, mat);
        if(cv::waitKey(200) >= 0) { ; }
    }

}
