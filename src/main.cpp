#include <iostream>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#if 0
typedef struct {
    uint16_t sync_word;
    uint16_t payload_length;
    uint16_t id;
    uint16_t col;
    uint16_t cols;
    uint16_t row;
    uint16_t rows;
    uint16_t crc;
} protocol_packet_header_t;
#endif

#define MAX_PACKET_SIZE (1200)
#define MAX_PACKETS (4096)

typedef struct {
    unsigned int head;
    unsigned int tail;
    unsigned int mask;
    unsigned int size;
    uint8_t buff_pool[MAX_PACKETS][MAX_PACKET_SIZE];
} image_packet_ring_t;

//unsigned int packet_head = 0;
//unsigned int packet_tail = 0;
//unsigned int packet_mask = (MAX_PACKETS - 1);
//uint8_t *packet_buff_pool[MAX_PACKETS][MAX_PACKET_SIZE];

void initialize_image_packet_ring(image_packet_ring_t *ring, unsigned int max_packets, unsigned int packet_size)
{
    memset(ring, 0, sizeof (image_packet_ring_t));
    ring->mask = max_packets - 1;
    ring->size = packet_size;
}

#define SUCCESS (0)
#define ERROR (1)

class ImagePacket {
public:
    uint32_t packet_counter;
    int send(void *payload, unsigned int length);
    int send(cv::Mat &mat);
    int send_image_row(uint8_t *row_data, unsigned int cols);
    image_packet_ring_t image_packet_ring;
    unsigned int run;
//    unsigned int rows;
//    unsigned int cols;
    ImagePacket();
};

ImagePacket::ImagePacket() {
    packet_counter = 0;
    run = 0;
    initialize_image_packet_ring(&image_packet_ring, MAX_PACKETS, MAX_PACKET_SIZE);
}

/*** transport layer interface ***/
int transport_ready(void) {
    return 0;
}

static void image_packet_looper(void *arg) {
    ImagePacket *img_pkt = (ImagePacket *) arg;
    fprintf(stderr, "image_packet_looper thread entering armed state\n");
    while (img_pkt->run == 0) {
    }
    fprintf(stderr, "image_packet_looper thread entering operational state\n");
    image_packet_ring_t *ring = &img_pkt->image_packet_ring;
    while (img_pkt->run == 1) {
        if (transport_ready()) {
            if (ring->tail != ring->head) {
                // img_pkt->send_image_row()
            }
        }
    }
}

int ImagePacket::send(void *payload, unsigned int length) {
}

#if 0
int ImagePacket::send_image_row(uint8_t *row_data, unsigned int cols) {
    unsigned int net_size = image_packet_ring.size - sizeof (protocol_packet_header_t);
    unsigned int n_entries = (cols + net_size - 1) / net_size;
    unsigned int tmp_head = image_packet_ring.head;
    int fits = 1;
    for (int i = 0; i < n_entries; ++i) {
        tmp_head = (tmp_head + 1) & image_packet_ring.mask;
        if (tmp_head == image_packet_ring.tail) {
            fits = 0;
            break;
        }
    }
    if (fits == 0) { return ERROR; }

    /* nicely fit the image row into ring buffer */
    unsigned int size_per_packet = (cols + n_entries - 1) / n_entries;
    unsigned int acc = 0;
    while (acc < cols) {
        unsigned int rem = cols - acc;
        int n = (rem > size_per_packet) ? size_per_packet : rem;
        protocol_packet_header_t *pkt_hdr = (protocol_packet_header_t *) image_packet_ring.buff_pool[image_packet_ring.head];
        uint8_t *payload_location = &image_packet_ring.buff_pool[image_packet_ring.head][sizeof (protocol_packet_header_t)];
        memcpy(payload_location, &row_data[acc], n);
        image_packet_ring.head = (image_packet_ring.head + 1) & image_packet_ring.mask;
        acc += n;
    }
}

int ImagePacket::send(cv::Mat &mat) {
    for (int i = 0; i < mat.rows; ++i) {
        uint8_t *row_data = mat.data; /* get row data */
        send_image_row(row_data, mat.cols);
    }
}

int send_image(protocol_packet_header_t *pkt_hdr, void *payload, unsigned int length)
{

}

#endif

double focus(cv::Mat &src) {
    cv::Mat src_gray, dst;
    int kernel_size = 3;
    int scale = 1;
    int delta = 0;
    int depth = CV_16S;

    GaussianBlur( src, src, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );
    cvtColor( src, src_gray, cv::COLOR_RGB2GRAY );
    // cv::Mat abs_dst;
    // Laplacian( src_gray, dst, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT );
    cv::Laplacian(src_gray, dst, CV_64F);
    cv::Scalar mu, sigma;
    cv::meanStdDev(dst, mu, sigma);

    cv::imshow("laplace", dst);

    double focusMeasure = sigma.val[0] * sigma.val[0];
    return focusMeasure;
}

extern "C" {
#include "udp.h"
}

/* frame grabber acts as a udp server */
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
            printf("received %d\n", n);
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
                    *focus = 1.111f; /* TODO */
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

int main(int argc, char **argv) {

    udp_server_t udp_server;
    memset(&udp_server, 0, sizeof (udp_server));
    udp_server.port = 55153;
    udp_server.run = 1;

    pthread_create(&udp_server.thread_id, NULL, udp_looper, &udp_server);

#if 0
    std::cout << "Hello, World!" << std::endl;

    printf("these are the %d arguments\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d] = \"%s\"", i, argv[i]);
    }
#endif

    cv::VideoCapture cap(2);
    if (!cap.isOpened()) { return -1; }

    cv::Mat edges;
    cv::Mat image;
    cv::namedWindow("edges", 1);
    cv::namedWindow("frame", 1);
    cv::namedWindow("laplace", 1);

    for(;;)
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
//        GaussianBlur(edges, edges, cv::Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
//        imshow("edges", edges);
        cv::imshow("edges", edges);
        cv::imshow("frame", frame);

        double focus_measure = focus(frame);
        // printf("focus = %f\n", focus_measure);

        if(cv::waitKey(200) >= 0) break;
    }

    return 0;
}
