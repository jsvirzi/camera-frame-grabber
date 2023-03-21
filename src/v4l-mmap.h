#ifndef V4L_MMAP_H
#define V4L_MMAP_H

#include <pthread.h>

#define MAX_V4L_BUFFERS (4)

typedef struct {
    unsigned int frame_number;
    unsigned int frame_max_count;
    unsigned int n_buffers;
    struct {
        void *start;
        size_t length;
    } buffers[MAX_V4L_BUFFERS];
    int fd;
    int io_method;
    const char *dev_name;
    int run;
    int rows;
    int cols;
    int bytes_per_pixel;
    void *image_process_stack;
    pthread_t thread_id;
    char thread_name[32];
    int thread_started;
} v4l_client;

#endif
