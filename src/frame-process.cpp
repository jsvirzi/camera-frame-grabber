#include <linux/videodev2.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
// #include "opencv2/core.hpp"

#include "frame-process.hpp"

extern "C" {
#include "graph-focus.h"
}

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

static ImageProcessStack *stack;

extern "C" {

#include "v4l-mmap.h"

    void frame_mouse_callback(int event, int x, int y, int flags, void *args)
    {
        FrameWindowParameters *params = (FrameWindowParameters *) args;
        if (event == cv::EVENT_LBUTTONDOWN)
        {
            params->pt2.x = x;
            params->pt2.y = y;
            params->roi_select_state = 0;
            params->l_click = 1;
            // fprintf(stderr, "left click at (%d, %d) with flags = %x\n", x, y, flags);
        }
        else if (event == cv::EVENT_RBUTTONDOWN)
        {
            params->pt1.x = x;
            params->pt1.y = y;
            params->roi_select_state = 1;
            params->r_click = 1;
        }
        else if (event == cv::EVENT_MBUTTONDOWN)
        {
            if (params->mid_click == 0) {
                params->mid_click_x = x;
                params->mid_click_y = y;
                params->mid_click = 1;
            }
            printf("middle button clicked\n");
        }
        else if (event == cv::EVENT_MOUSEMOVE)
        {
            if (params->roi_select_state) {
                params->pt2.x = x;
                params->pt2.y = y;
            }
        }
    }

    /*
     * @brief
     *
     * @param src is grey scale mat
     */
    double focus(cv::Mat &src) {
        cv::Mat src_gray, dst;
        int kernel_size = 3;
        int scale = 1;
        int delta = 0;
        int depth = CV_16S;

        GaussianBlur( src, src, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );
        src_gray = src;
        cv::Mat abs_dst;
        // cv::Laplacian( src_gray, dst, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT );
        cv::Laplacian(src_gray, dst, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(dst, mu, sigma);

        // cv::imshow("laplace", dst);

        double focusMeasure = sigma.val[0] * sigma.val[0];
        return focusMeasure;
    }

    static int delay_us(unsigned int microseconds)
    {
        struct timeval tv;
        tv.tv_sec = microseconds / 1000000ULL;
        tv.tv_usec = microseconds - 1000000ULL * tv.tv_sec;
        select(1, NULL, NULL, NULL, &tv);
        return 0;
    }

    int save_file(cv::Mat &mat, const char *filename) {
        size_t size = mat.rows * mat.cols;
        int fd = open(filename, O_RDWR | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR);
        size_t rem = size;
        uint8_t *p = mat.data;
        while (rem) {
            size_t n = write(fd, p, rem);
            rem = rem - n;
            p = p + n;
        }
        close(fd);
        printf("saving file %s with %zd bytes\n", filename, size);
    }

    void *image_process_looper(void *arg)
    {
        ImageProcessStack *stack = (ImageProcessStack *) arg;
        while (stack->run) {
            if (stack->buff_head == stack->buff_tail) { delay_us(1000); }
            unsigned int index = (stack->buff_head - 1) & stack->buff_mask;
            cv::Mat mat(stack->rows, stack->cols, CV_8U, stack->buff[index]);
            for (int y = 0; y < stack->rows; y += stack->roi_stride_y) {
                cv::Point pt1(0, y);
                cv::Point pt2(stack->cols - 1, y);
                cv::line(mat, pt1, pt2, (0, 255, 0), 2);
            }
            for (int x = 0; x < stack->cols; x += stack->roi_stride_x) {
                cv::Point pt1(x, 0);
                cv::Point pt2(x, stack->rows - 1);
                cv::line(mat, pt1, pt2, (0, 255, 0), 2);
            }

            std::vector<cv::Rect> mCells;
            index = 0;
            for (int y = 0; y < stack->rows; y += stack->roi_stride_y) {
                for (int x = 0; x < stack->cols; x += stack->roi_stride_x) {
                    cv::Rect grid_rect(x, y, stack->roi_stride_x, stack->roi_stride_y);
                    // std::cout << grid_rect<< std::endl;
                    mCells.push_back(grid_rect);
                    rectangle(mat, grid_rect, cv::Scalar(0, 255, 0), 1);
                    cv::Mat roi(mat, grid_rect);
                    // imshow(cv::format("grid-%d-%d", x, y), roi);
                    stack->focus_measure[index] = focus(roi);
                    // printf("focus(%d,%d) = %f/%d\n", y, x, stack->focus_measure[index], index);
                    ++index;
                }
            }

            stack->focus_measure_img = focus(mat);
            stack->focus_measure[index++] = stack->focus_measure_img;

            if (stack->frame_window_params.mid_click) {
                char filename[256];
                ++stack->filename_cntr;
                snprintf(filename, sizeof (filename), "%s-%d.raw", stack->filename_base, stack->filename_cntr);
                save_file(mat, filename);
                stack->frame_window_params.mid_click = 0;
            }

            cv::Point pt1(stack->frame_window_params.pt1.x, stack->frame_window_params.pt1.y);
            cv::Point pt2(stack->frame_window_params.pt2.x, stack->frame_window_params.pt2.y);
            cv::Scalar color(0, 255, 0);
            int thickness = 2;
            cv::rectangle(mat, pt1, pt2, color, thickness);

            int min_x = (pt1.x <= pt2.x) ? pt1.x : pt2.x;
            int max_x = (pt1.x > pt2.x) ? pt1.x : pt2.x;
            int min_y = (pt1.y <= pt2.y) ? pt1.y : pt2.y;
            int max_y = (pt1.y > pt2.y) ? pt1.y : pt2.y;
            int len_x = max_x - min_x;
            int len_y = max_y - min_y;

            int roi_good = (max_x > 0) && (max_y > 0) && (len_x > 0) && (len_y > 0) && stack->frame_window_params.l_click && stack->frame_window_params.r_click;

            if (roi_good) {
                cv::Rect roi_rect(min_x, min_y, len_x, len_y);
                cv::Mat roi(mat, roi_rect);
                stack->focus_measure_roi = focus(roi);
                stack->focus_measure[index++] = stack->focus_measure_roi;
                // imshow("roi", roi);
            }

            cv::imshow(stack->window_name, mat);

            display_focus_graph(stack->focus_measure);

            if(cv::waitKey(100) >= 0) { ; }
            stack->buff_tail = (stack->buff_tail + 1) & stack->buff_mask;
        }
    }

    int image_process_stack_deinitialize(void)
    {
        for (int i = 0; i <= stack->buff_mask; ++i) { delete [] stack->buff[i]; }
        delete [] stack->buff;
    }

    int image_process_stack_initialize(v4l_client *client, const char *filename_base)
    {
        unsigned int rows = client->rows;
        unsigned int cols = client->cols;
        stack = new ImageProcessStack;
        stack->n_roi_x = client->n_roi_x;
        stack->n_roi_y = client->n_roi_y;
        stack->window_name = "image";
        cv::namedWindow(stack->window_name, 1);
        stack->rows = rows;
        stack->cols = cols;
        stack->pixel_format = client->pixel_format;
        stack->buff_size = rows * cols * 2; /* TODO will this always suffice? */
        /* TODO reduce number of buffers to 4 */
#define N_BUFFERS (8)
        stack->buff_mask = N_BUFFERS - 1;
        stack->buff_head = 0;
        stack->buff_tail = 0;
        stack->buff = new uint8_t * [N_BUFFERS];
        for (int i = 0; i < N_BUFFERS; ++i) {
            stack->buff[i] = new uint8_t [stack->buff_size];
        }
        stack->roi_index = new uint8_t [stack->buff_size];
        stack->roi_stride_x = (cols + stack->n_roi_x - 1) / stack->n_roi_x;
        stack->roi_stride_y = (rows + stack->n_roi_y - 1) / stack->n_roi_y;
        for (int row = 0; row < rows; ++row) {
            int j = row / stack->roi_stride_y;
            for (int col = 0; col < cols; ++col) {
                int i = col / stack->roi_stride_x;
                stack->roi_index[row * cols + col] = (j * stack->n_roi_x) + i;
            }
        }

        unsigned int n_roi = stack->n_roi_x * stack->n_roi_y;
        stack->roi_buff = new uint8_t * [n_roi];
        stack->roi_buff_size = stack->roi_stride_x * stack->roi_stride_y;

        stack->roi_buff_index = new unsigned int [n_roi];
        int k = 0;
        for (int i = 0; i < stack->n_roi_x; ++i) {
            for (int j = 0; j < stack->n_roi_y; ++j, ++k) {
                stack->roi_buff[k] = new uint8_t [stack->roi_buff_size];
            }
        }

        /* one focus measure for each roi + 1 for whole image + 1 for custom roi */
        stack->focus_measure = new double [stack->n_roi_x * stack->n_roi_y];

        memset(&stack->frame_window_params, 0, sizeof (stack->frame_window_params));
        cv::setMouseCallback(stack->window_name, frame_mouse_callback, &stack->frame_window_params);

        snprintf(stack->filename_base, sizeof (stack->filename_base), "%s", filename_base);
        stack->filename_cntr = 0;

        stack->run = 1;
        int err = pthread_create(&stack->thread_id, 0, image_process_looper, stack);
        return 0;
    }

    void image_process_stack_process_image(void const * const data, unsigned int size)
    {
        static int counter = 0;
        ++counter;
        // if ((counter % 10) == 0) { printf("image size = %d\n", size); }

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
            stack->buff_head = new_head;
        }
    }

}
