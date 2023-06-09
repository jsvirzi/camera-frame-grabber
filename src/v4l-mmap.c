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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "v4l-mmap.h"
#include "udp.h"
#include "graph-focus.h"

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

// #define MAX_BUFFERS (4)

#define N_ROI_X (4)
#define N_ROI_Y (4)
// #define N_ROI (N_ROI_X * N_ROI_Y)

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

int image_process_stack_initialize(v4l_client *client, const char *filename_base); /* TODO move into include file */
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

void *udp_looper(void *arg)
{
    udp_server_t *server = (udp_server_t *) arg;

    server->client_addr_len = sizeof (server->client_addr);

    server->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->socket_fd < 0) {
        fprintf(stderr, "unable to open udp socket\n");
        return 0;
    }

    int optval = 1; /* allows rerun server immediately after killing it; avoid waiting for system to figure it out */
    setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));

    /* build the server's Internet address */
    memset(&server->server_addr, 0, sizeof(server->server_addr));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server->server_addr.sin_port = htons((unsigned short) server->port);

    if (bind(server->socket_fd, (struct sockaddr *) &server->server_addr, sizeof(server->server_addr)) < 0) {
        fprintf(stderr, "bind: error");
        return 0;
    }

    while (server->run) {
        int status = check_socket(server->socket_fd);
        if (status) {
            socklen_t len = sizeof (server->client_addr);
            int n = recvfrom(server->socket_fd, &server->incoming_packet, sizeof (server->incoming_packet), 0,
                (struct sockaddr *) &server->client_addr, &len);
            if (n == sizeof (protocol_packet_header_t)) {
                switch (server->incoming_packet.header.type) {
                case PACKET_TYPE_REQUEST_FOCUS: {
                    protocol_packet_header_t *header = &server->outgoing_packet.header;
                    header->sync_word = SYNC_WORD;
                    header->type = PACKET_TYPE_RESPOND_FOCUS;
                    header->length = sizeof (float);
                    header->serial_number = server->serial_number;
                    ++server->serial_number;
                    float *focus = (float *) server->outgoing_packet.payload;
                    *focus = 1.111; /* TODO */
                    int n = sizeof (protocol_packet_header_t) + server->outgoing_packet.header.length;
                    sendto(server->socket_fd, &server->outgoing_packet, n, 0, (struct sockaddr *) &server->client_addr, server->client_addr_len);
                    break;
                }
                }
            }
        }
    }
    return 0;
}

#if 0
void *udp_looper(void *arg)
{
    udp_server_t *server = (udp_server_t *) arg;

    server->client_addr_len = sizeof (server->client_addr);

    server->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->socket_fd < 0) {
        fprintf(stderr, "unable to open udp socket\n");
        return 0;
    }

    int optval = 1; /* allows rerun server immediately after killing it; avoid waiting for system to figure it out */
    setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));

    /* build the server's Internet address */
    memset(&server->server_data_addr, 0, sizeof(server->server_data_addr));
    server->server_data_addr.sin_family = AF_INET;
    server->server_data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server->server_data_addr.sin_port = htons((unsigned short) server->port);

    if (bind(server->socket_fd, (struct sockaddr *) &server->server_data_addr, sizeof(server->server_data_addr)) < 0) {
        fprintf(stderr, "bind: error");
        return 0;
    }

    while (server->run) {
        int status = check_socket(server->socket_fd);
        if (status) {
            socklen_t len = sizeof (server->client_addr);
            int n = recvfrom(server->socket_fd, &server->incoming_data_packet, sizeof (server->incoming_data_packet), 0,
                (struct sockaddr *) &server->client_addr, &len);
            if (n == sizeof (protocol_packet_header_t)) {
                switch (server->incoming_data_packet.header.type) {
                case PACKET_TYPE_REQUEST_FOCUS: {
                    protocol_packet_header_t *header = &server->outgoing_data_packet.header;
                    header->sync_word = SYNC_WORD;
                    header->type = PACKET_TYPE_RESPOND_FOCUS;
                    header->length = sizeof (float);
                    header->serial_number = server->serial_number;
                    ++server->serial_number;
                    float *focus = (float *) server->outgoing_data_packet.payload;
                    *focus = 1.111; /* TODO */
                    int n = sizeof (protocol_packet_header_t) + server->outgoing_data_packet.header.length;
                    sendto(server->socket_fd, &server->outgoing_data_packet, n, 0, (struct sockaddr *) &server->client_addr, server->client_addr_len);
                    break;
                }
                }
            }
        }
    }
    return 0;
}

#endif

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
    if (fmt.fmt.pix.bytesperline < min) { fmt.fmt.pix.bytesperline = min; }
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min) { fmt.fmt.pix.sizeimage = min; }

    printf("pixel format = 0x%8.8x\n", client->pixel_format);
    if (client->pixel_format == 0) {
        struct v4l2_fmtdesc fmtdesc;
        memset(&fmtdesc,0,sizeof(fmtdesc));
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        printf("available:\n");
        while (ioctl(client->fd,VIDIOC_ENUM_FMT, &fmtdesc) == 0)
        {
            printf("format: %s => 0x%8.8x index = %d type = %d flags = 0x%8.8x\n",
                fmtdesc.description, fmtdesc.pixelformat, fmtdesc.index, fmtdesc.type, fmtdesc.flags);
            fmtdesc.index++;
        }
        return SUCCESS;
    }

    client->bytes_per_pixel = fmt.fmt.pix.sizeimage / (client->cols * client->rows);
    printf("pixel width/height = (%d, %d) = %d. size = %d\n", client->cols, client->rows, client->cols * client->rows, fmt.fmt.pix.sizeimage);
    uint32_t word = client->pixel_format;
    printf("pixel format = %c%c%c%c\n", (word >> 0x18) & 0xff, (word >> 0x10) & 0xff, (word >> 0x08) & 0xff, (word >> 0x00) & 0xff);
    memset(&fmt, 0, sizeof (fmt));
    client->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.type = client->type;
    fmt.fmt.pix.width = client->cols;
    fmt.fmt.pix.height = client->rows;
    fmt.fmt.pix.pixelformat = client->pixel_format;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (xioctl(client->fd, VIDIOC_S_FMT, &fmt) < 0) {
        printf("fail: VIDIO_S_FMT");
        return ERROR;
    }

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

    if (req.count < 2)
    {
        fprintf(stderr, "Insufficient buffer memory on %s\n", client->dev_name);
        return FAILURE;
    }

    for (client->n_buffers = 0; client->n_buffers < req.count; ++client->n_buffers)
    {
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
    char filename[16];
    snprintf(filename, sizeof (filename), "frame-%d.raw", client->frame_number);
//    FILE *fp = fopen(filename,"wb");
//    fwrite(p, size, 1, fp);
//    fflush(fp);
//    fclose(fp);
    return SUCCESS;
}

static int read_frame(v4l_client *client)
{
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof (buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (client->io_method == IO_METHOD_MMAP) {
        buf.memory = V4L2_MEMORY_MMAP;
        assert(buf.index < client->n_buffers);
        unsigned int n = (buf.bytesused > 0) ? buf.bytesused : buf.length;
        // printf("process_image(%p, %p, %d)\n", client, client->buffers[buf.index].start, n);
        process_image(client, client->buffers[buf.index].start, n);
        if (xioctl(client->fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) { return FAILURE; }
            else { return ERROR; }
        }
    } else if (client->io_method == IO_METHOD_USERPTR) {
        buf.memory = V4L2_MEMORY_USERPTR;
        process_image(client, (void *) buf.m.userptr, buf.bytesused);
        if (xioctl(client->fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) { return SUCCESS; }
            else { return FAILURE; }
        }
        unsigned int i;
        for (i = 0; i < client->n_buffers; ++i) {
            if (buf.m.userptr == (unsigned long) client->buffers[i].start && buf.length == client->buffers[i].length) { break; }
        }
        assert(i < client->n_buffers);
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
            }

            if (read_frame(client) == SUCCESS) { break; }
        }

        ++client->frame_number;
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
    image_process_stack_initialize(client, client->filename_base);
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

int main(int argc, char **argv)
{
    v4l_client client;
    memset(&client, 0, sizeof (client));
    client.n_roi_x = N_ROI_X;
    client.n_roi_y = N_ROI_Y;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-res") == 0) {
            client.cols = atoi(argv[++i]);
            client.rows = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-roi") == 0) {
            client.n_roi_x = atoi(argv[++i]);
            client.n_roi_y = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-yuyv") == 0) {
            client.pixel_format = V4L2_PIX_FMT_YUYV;
        } else if (strcmp(argv[i], "-uyvy") == 0) {
            client.pixel_format = V4L2_PIX_FMT_UYVY;
        } else if (strcmp(argv[i], "-grey") == 0) {
            client.pixel_format = V4L2_PIX_FMT_GREY;
        } else if (strcmp(argv[i], "-day") == 0) {
            client.pixel_format = V4L2_PIX_FMT_UYVY;
            client.n_roi_x = 1600;
            client.n_roi_y = 1300;
        } else if (strcmp(argv[i], "-night") == 0) {
            client.pixel_format = V4L2_PIX_FMT_UYVY;
            client.n_roi_x = 1600;
            client.n_roi_y = 1300;
        } else if (strcmp(argv[i], "-context") == 0) {
            client.pixel_format = V4L2_PIX_FMT_UYVY;
            client.n_roi_x = 1920;
            client.n_roi_y = 1080;
        } else if (strcmp(argv[i], "-fmt") == 0) {
            sscanf(argv[++i], "%x", &client.pixel_format);
        } else if (strcmp(argv[i], "-d") == 0) {
            snprintf(client.dev_name, sizeof (client.dev_name), "%s", argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0) {
            client.udp_data_port = atoi(argv[++i]);
            client.udp_ctrl_port = atoi(argv[++i]);
            client.udp_http_port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-name") == 0) {
            snprintf(client.filename_base, sizeof (client.filename_base), "%s", argv[++i]);
        }
    }

    initialize_focus_graph(client.n_roi_x * client.n_roi_y + 2);

    printf("supported formats:\n");
    printf("uyvy = 0x%8.8x\n", V4L2_PIX_FMT_UYVY);
    printf("yuyv = 0x%8.8x\n", V4L2_PIX_FMT_YUYV);
    printf("grey = 0x%8.8x\n", V4L2_PIX_FMT_GREY);
    printf("\n");
    printf("pixel: (cols x rows) = (%d x %d). format = %x", client.cols, client.rows, client.pixel_format);
    printf("roi: %d(H) x %d(V)\n", client.n_roi_x, client.n_roi_y);
    printf("name: [%s]\n", client.filename_base);

    open_device(&client);
    init_device(&client);
    arm_capture(&client);
    init_stack(&client);

    udp_server_t udp_server;
    memset(&udp_server, 0, sizeof (udp_server));
    udp_server.port = client.udp_data_port;
    udp_server.run = 1;

    pthread_create(&udp_server.thread_id, NULL, udp_looper, &udp_server);

    client.run = 1;

    unsigned int last_report = client.frame_number;
    while (client.run) {
        if ((client.frame_number % 100) == 0) {
            if (client.frame_number != last_report) {
                fprintf(stdout, "%d frames\n", client.frame_number);
                last_report = client.frame_number;
            }
        }
    }

    client.run = 0;
    udp_server.run = 0;
    pthread_join(udp_server.thread_id, 0);

    stop_capturing(&client);
    uninit_device(&client);
    close_device(&client);

    return 0;
}
