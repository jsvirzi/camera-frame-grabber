#ifndef V4L_MMAP_H
#define V4L_MMAP_H

#include <pthread.h>

#define MAX_V4L_BUFFERS (4)

typedef struct {
    unsigned int frame_number;
    unsigned int n_buffers;
    struct {
        void *start;
        size_t length;
    } buffers[MAX_V4L_BUFFERS];
    int fd;
    int io_method;
    char dev_name[64];
    int run;
    int rows;
    int cols;
    int type;
    int udp_data_port;
    int udp_ctrl_port;
    int udp_http_port;
    int bytes_per_pixel;
    void *image_process_stack;
    pthread_t thread_id;
    char thread_name[32];
    int thread_started;
    uint32_t pixel_format;
    int n_roi_x;
    int n_roi_y;
} v4l_client;

#endif
