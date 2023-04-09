#include <linux/videodev2.h>
#include <pthread.h>

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

    static int delay_us(unsigned int microseconds)
    {
        struct timeval tv;
        tv.tv_sec = microseconds / 1000000ULL;
        tv.tv_usec = microseconds - 1000000ULL * tv.tv_sec;
        select(1, NULL, NULL, NULL, &tv);
        return 0;
    }

    void *image_process_looper(void *arg)
    {
        ImageProcessStack *stack = (ImageProcessStack *) arg;
        while (stack->run) {
            if (stack->buff_head == stack->buff_tail) { delay_us(1000); }
            unsigned int index = (stack->buff_head - 1) & stack->buff_mask;
            cv::Mat mat(stack->rows, stack->cols, CV_8U, stack->buff[index]);
            cv::imshow(stack->window_name, mat);
            if(cv::waitKey(100) >= 0) { ; }
            stack->buff_tail = (stack->buff_tail + 1) & stack->buff_mask;
        }
    }

    int image_process_stack_deinitialize(void)
    {
        for (int i = 0; i <= stack->buff_mask; ++i) { delete [] stack->buff[i]; }
        delete [] stack->buff;
    }

    int image_process_stack_initialize(unsigned int rows, unsigned int cols, unsigned int format)
    {
        stack = new ImageProcessStack;
        stack->window_name = "image";
        cv::namedWindow(stack->window_name, 1);
        stack->rows = rows;
        stack->cols = cols;
        stack->pixel_format = format;
        stack->buff_size = rows * cols * 4; /* TODO will this always suffice? */
#define N_BUFFERS (8)
        stack->buff_mask = N_BUFFERS - 1;
        stack->buff_head = 0;
        stack->buff_tail = 0;
        stack->buff = new uint8_t * [N_BUFFERS];
        for (int i = 0; i < N_BUFFERS; ++i) {
            stack->buff[i] = new uint8_t [stack->buff_size];
        }

        stack->run = 1;
        int err = pthread_create(&stack->thread_id, 0, image_process_looper, stack);
        return 0;
    }

    void image_process_stack_process_image(void const * const data, unsigned int size)
    {
        static int counter = 0;
        ++counter;
        if ((counter % 10) == 0) {
            printf("image size = %d\n", size);
        }

        uint8_t *img_data = (uint8_t *) data;
        unsigned int new_head = (stack->buff_head + 1) & stack->buff_mask;
        if (new_head == stack->buff_tail) { return; }
        uint8_t *buff = stack->buff[stack->buff_head];
        unsigned int index = 0;
        if (stack->pixel_format == V4L2_PIX_FMT_YUYV) {
            /* uyvy 422 format */
            for (int i = 0; i < stack->rows; ++i) {
                uint8_t *p = (uint8_t *) &img_data[i * stack->cols * 2];
                for (int j = 0; j < stack->cols; j += 2) {
                    uint8_t ux = *p++;
                    uint8_t y0 = *p++;
                    uint8_t vx = *p++;
                    uint8_t y1 = *p++;
                    buff[index++] = y0;
                    buff[index++] = y1;
                }
            }
            stack->buff_head = new_head;
        } else if (stack->pixel_format == V4L2_PIX_FMT_GREY) {
            /* uyvy 422 format */
            for (int i = 0; i < stack->rows; ++i) {
                uint8_t *p = (uint8_t *) &img_data[i * stack->cols];
                for (int j = 0; j < stack->cols; ++j) {
                    uint8_t y = *p++;
                    buff[index++] = y;
                }
            }
            cv::Mat mat(stack->rows, stack->cols, CV_8U, buff);
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
