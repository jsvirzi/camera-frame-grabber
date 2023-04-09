#include <linux/videodev2.h>

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

    int image_process_stack_initialize(unsigned int rows, unsigned int cols, unsigned int format)
    {
        stack = new ImageProcessStack;
        stack->window_name = "image";
        cv::namedWindow(stack->window_name, 1);
        stack->rows = rows;
        stack->cols = cols;
        stack->pixel_format = format;
        stack->buff_size = rows * cols * 4; /* TODO will this always suffice? */
        stack->buff = new uint8_t [stack->buff_size];
        return 0;
    }

    void image_process_stack_process_image(void const * const data, unsigned int size)
    {
        static int counter = 0;
        ++counter;
        if ((counter % 10) == 0) {
            printf("image size = %d\n", size);
        }

        if (stack->pixel_format == V4L2_PIX_FMT_YUYV) {
            /* uyvy 422 format */
            uint8_t *buff = stack->buff;
            unsigned int index = 0;
            unsigned int cols = stack->cols;
            unsigned int rows = stack->rows;
            uint8_t *img_data = (uint8_t *) data;

            for (int i = 0; i < rows; ++i) {
                uint8_t *p = (uint8_t *) &img_data[i * cols * 2];
                for (int j = 0; j < cols; j += 2) {
                    uint8_t ux = *p++;
                    uint8_t y0 = *p++;
                    uint8_t vx = *p++;
                    uint8_t y1 = *p++;
                    buff[index++] = y0;
                    buff[index++] = y1;
                }
            }
            cv::Mat mat(rows, cols, CV_8U, buff);
            cv::imshow(stack->window_name, mat);
            if(cv::waitKey(200) >= 0) { ; }
        }

        // cv::Mat mat(stack->rows, stack->cols, type, data);

#if 0
        uint8_t *buff = (uint8_t *) data;
        cv::Mat mat(stack->rows, stack->cols, CV_8U, buff);
#endif

        // cv::Mat mat(stack->rows, stack->cols, CV_8U, (uint8_t *) data);
    }

}
