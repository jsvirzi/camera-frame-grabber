/*
 * V4L2 video capture example
 *
 * This program can be used and distributed without restrictions.
 *
 * This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 *
 * https://gist.github.com/maxlapshin/1253534
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdint.h>

#include "v4l-mmap.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#ifndef V4L2_PIX_FMT_H264
#define V4L2_PIX_FMT_H264     v4l2_fourcc('H', '2', '6', '4') /* H264 with start codes */
#endif

#ifndef SUCCESS
#define SUCCESS (EXIT_SUCCESS)
#endif

#ifndef FAILURE
#define FAILURE (EXIT_FAILURE)
#endif

#ifndef ERROR
#define ERROR (-1)
#endif

#define MAX_BUFFERS (4)

enum io_method {
    IO_METHOD_MMAP = 0,
    IO_METHOD_USERPTR
};

static int init_device(v4l_client *client);
static int init_userp(v4l_client *client, unsigned int buffer_size);
static int init_mmap(v4l_client *client);
static int open_device(v4l_client *client);
static int stop_capturing(v4l_client *client);
static int uninit_device(v4l_client *client);
static int close_device(v4l_client *client);
static int init_stack(v4l_client *client);
static int close_stack(v4l_client *client);

int image_process_stack_initialize(unsigned int rows, unsigned int cols, unsigned int type);
void image_process_stack_process_image(void const * const data, unsigned int size);

static int delay_us(unsigned int microseconds)
{
    struct timeval tv;
    tv.tv_sec = microseconds / 1000000ULL;
    tv.tv_usec = microseconds - 1000000ULL * tv.tv_sec;
    select(1, NULL, NULL, NULL, &tv);
    return SUCCESS;
}

static int errno_exit(const char *s)
{
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    return FAILURE;
}

static int xioctl(int fh, int request, void *arg)
{
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while ((r == -1) && (errno == EINTR));
    return r;
}

static int open_device(v4l_client *client)
{
    int error_code = -1;
    struct stat st;

    if (stat(client->dev_name, &st) < 0) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", client->dev_name, errno, strerror(errno));
        return error_code;
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", client->dev_name);
        return error_code;
    }

    client->fd = open(client->dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (client->fd < 0) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", client->dev_name, errno, strerror(errno));
        return error_code;
    }

    return client->fd;
}

static int init_device(v4l_client *client)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;

    if (xioctl(client->fd, VIDIOC_QUERYCAP, &cap) < 0) {
        if (errno == EINVAL) {
            fprintf(stderr, "%s is not a valid V4L2 device\n", client->dev_name);
            return FAILURE;
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
        fprintf(stderr, "%s is not a video capture device\n", client->dev_name);
        return FAILURE;
    }

    if ((cap.capabilities & V4L2_CAP_STREAMING) == 0) {
        fprintf(stderr, "%s does not support streaming i/o\n", client->dev_name);
        return FAILURE;
    }

    /* Select video input, video standard and tune here. */

    memset(&cropcap, 0, sizeof (cropcap));

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(client->fd, VIDIOC_CROPCAP, &cropcap) == 0) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (xioctl(client->fd, VIDIOC_S_CROP, &crop) < 0) {
            printf("cropping not supported\n");
        }
    }

    memset(&fmt, 0, sizeof (fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(client->fd, VIDIOC_G_FMT, &fmt) < 0) { errno_exit("VIDIOC_G_FMT"); };

    /* TODO why? buggy driver paranoia. */
    unsigned int min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    uint32_t word = fmt.fmt.pix.pixelformat;
    printf("pixel format = %c%c%c%c\n", (word >> 0x18) & 0xff, (word >> 0x10) & 0xff, (word >> 0x08) & 0xff, (word >> 0x00) & 0xff);
    client->type = fmt.type;
    client->rows = fmt.fmt.pix.height;
    client->cols = fmt.fmt.pix.width;
    client->bytes_per_pixel = fmt.fmt.pix.sizeimage / (client->cols * client->rows);

    if (client->io_method == IO_METHOD_MMAP) {
        init_mmap(client);
    } else if (client->io_method == IO_METHOD_USERPTR) {
        init_userp(client, fmt.fmt.pix.sizeimage);
    }

    return SUCCESS;
}

static int arm_capture(v4l_client *client)
{
    for (unsigned int i = 0; i < client->n_buffers; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.index = i;

        if (client->io_method == IO_METHOD_MMAP) {
            buf.memory = V4L2_MEMORY_MMAP;
        } else if (client->io_method == IO_METHOD_USERPTR) {
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.m.userptr = (unsigned long) client->buffers[i].start;
            buf.length = client->buffers[i].length;
        }

        if (xioctl(client->fd, VIDIOC_QBUF, &buf) < 0) { errno_exit("VIDIOC_QBUF"); }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(client->fd, VIDIOC_STREAMON, &type) < 0) { errno_exit("VIDIOC_STREAMON"); }

    return SUCCESS;
}

static int init_mmap(v4l_client *client)
{
    struct v4l2_requestbuffers req;

    memset(&req, 0, sizeof (req));

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(client->fd, VIDIOC_REQBUFS, &req) < 0) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support memory mapping\n", client->dev_name);
            return FAILURE;
        } else { errno_exit("VIDIOC_REQBUFS"); }
    }

    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", client->dev_name);
        return FAILURE;
    }

    for (client->n_buffers = 0; client->n_buffers < req.count; ++client->n_buffers) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof (buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = client->n_buffers;

        if (xioctl(client->fd, VIDIOC_QUERYBUF, &buf) < 0) { errno_exit("VIDIOC_QUERYBUF"); }

        client->buffers[client->n_buffers].length = buf.length;
        client->buffers[client->n_buffers].start = mmap(NULL /* start anywhere */,
            buf.length,
            PROT_READ | PROT_WRITE /* required */,
            MAP_SHARED /* recommended */,
            client->fd,
            buf.m.offset);

        if (client->buffers[client->n_buffers].start == MAP_FAILED) { errno_exit("mmap"); }
    }

    return SUCCESS;
}

static int init_userp(v4l_client *client, unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count  = 4;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (xioctl(client->fd, VIDIOC_REQBUFS, &req) < 0) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support user pointer i/o\n", client->dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    for (client->n_buffers = 0; client->n_buffers < 4; ++client->n_buffers) {
        client->buffers[client->n_buffers].length = buffer_size;
        client->buffers[client->n_buffers].start = malloc(buffer_size);

        if (client->buffers[client->n_buffers].start == 0) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

static int process_image(v4l_client *client, void const * const p, int size)
{
    ++client->frame_number;
    image_process_stack_process_image(p, size);
    char filename[15];
    sprintf(filename, "frame-%d.raw", client->frame_number);
    FILE *fp = fopen(filename,"wb");
    fwrite(p, size, 1, fp);
    fflush(fp);
    fclose(fp);
    return SUCCESS;
}

static int read_frame(v4l_client *client)
{
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof (buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (client->io_method == IO_METHOD_MMAP) {
        buf.memory = V4L2_MEMORY_MMAP;
        if (xioctl(client->fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) { return FAILURE; }
            else { return ERROR; }
        }
        assert(buf.index < client->n_buffers);
        process_image(client, client->buffers[buf.index].start, buf.bytesused);
    } else if (client->io_method == IO_METHOD_USERPTR) {
        buf.memory = V4L2_MEMORY_USERPTR;
        if (xioctl(client->fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) { return SUCCESS; }
            else { return FAILURE; }
        }
        unsigned int i;
        for (i = 0; i < client->n_buffers; ++i) {
            if (buf.m.userptr == (unsigned long) client->buffers[i].start && buf.length == client->buffers[i].length) { break; }
        }
        assert(i < client->n_buffers);
        process_image(client, (void *) buf.m.userptr, buf.bytesused);
    }
    if (xioctl(client->fd, VIDIOC_QBUF, &buf) < 0) { errno_exit("VIDIOC_QBUF"); }

    return SUCCESS;
}

static void *v4l_looper(void *arg)
{
    v4l_client *client = (v4l_client *) arg;

    client->thread_started = 1;

    while (client->run == 0) {
        delay_us(100);
    }

    while (client->run == 1) {

        while (1) {
            fd_set fds;
            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            FD_ZERO(&fds);
            FD_SET(client->fd, &fds);

            int r = select(client->fd + 1, &fds, NULL, NULL, &tv);

            if (r == -1) {
                if (EINTR == errno) { continue; }
                errno_exit("select");
            } else if (r == 0) {
                fprintf(stderr, "select timeout\n");
                continue;
                // exit(EXIT_FAILURE);
            }

            if (read_frame(client) == SUCCESS) { break; }
        }

        ++client->frame_number;
        if (client->frame_max_count && (client->frame_number > client->frame_max_count)) { break; }
        /* EAGAIN - continue select loop. */

    }

    client->thread_started = 0;

    return NULL;
}

static int stop_capturing(v4l_client *client)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(client->fd, VIDIOC_STREAMOFF, &type) < 0) {
        errno_exit("VIDIOC_STREAMOFF");
    }
    return SUCCESS;
}

static int close_device(v4l_client *client)
{
    if (close(client->fd) < 0) { errno_exit("close"); }
    return SUCCESS;
}

static int uninit_device(v4l_client *client)
{
    unsigned int i;

    if (client->io_method == IO_METHOD_MMAP) {
        for (i = 0; i < client->n_buffers; ++i) {
            if (munmap(client->buffers[i].start, client->buffers[i].length) < 0) {
                errno_exit("munmap");
            }
        }
    } else if (client->io_method == IO_METHOD_USERPTR) {
        for (i = 0; i < client->n_buffers; ++i) {
            free(client->buffers[i].start);
        }
    }

    return SUCCESS;
}

static int init_stack(v4l_client *client)
{
    image_process_stack_initialize(client->rows, client->cols, client->type);
    client->run = 0;
    client->thread_started = 0;
    int status = pthread_create(&client->thread_id, NULL, v4l_looper, client);
    if (status != 0) {
        fprintf(stderr, "unable to create serial thread\n");
        return FAILURE;
    }
    snprintf(client->thread_name, sizeof (client->thread_name), "v4l-mmap");
    return SUCCESS;
}

static int close_stack(v4l_client *client)
{
    void *exit_status;
    client->run = 0;
    while (client->thread_started) { delay_us(100); }
    pthread_join(client->thread_id, &exit_status);
    return SUCCESS;
}

int main(int argc, char **argv) {
    v4l_client client;
    memset(&client, 0, sizeof (client));
    client.frame_max_count = 200;
    client.dev_name = "/dev/video2";
    open_device(&client);
    init_device(&client);
    arm_capture(&client);
    init_stack(&client);

    client.run = 1;

    client.frame_max_count = 0;
    unsigned int last_report = client.frame_number;
    while ((client.frame_max_count == 0) || client.frame_number < client.frame_max_count) {
        if ((client.frame_number % 10) == 0) {
            if (client.frame_number != last_report) {
                fprintf(stdout, "%d frames\n", client.frame_number);
                last_report = client.frame_number;
            }
        }
    }

    stop_capturing(&client);
    uninit_device(&client);
    close_device(&client);

    return 0;
}


#if 0

int main(int argc, char **argv)
{
    open_device();
    init_device();
    start_capturing();
    mainloop();
    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\n");
    return 0;
}

#endif
