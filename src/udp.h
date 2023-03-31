#ifndef UDP_H
#define UDP_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <pthread.h>

typedef struct {
    uint16_t sync_word;
    uint16_t length;
    uint16_t type;
    uint16_t id;
    uint32_t time;
    uint16_t serial_number;
    uint16_t crc;
} protocol_packet_header_t;

#define MAX_PAYLOAD_SIZE (1024)

typedef struct {
    protocol_packet_header_t header;
    uint8_t payload[MAX_PAYLOAD_SIZE];
} protocol_packet_t;

typedef struct {
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int socket_fd;
    int port;
    int run;
    uint8_t ip_addr[4];
    protocol_packet_t incoming_packet;
    protocol_packet_t outgoing_packet;
    socklen_t client_addr_len;
    size_t serial_number;
    pthread_t thread_id;
} udp_server_t;

unsigned int check_socket(int socket_fd);

#define SYNC_WORD (0xCAFE)
#define DEFAULT_UDP_PORT (55153)

enum {
    PACKET_TYPE_REQUEST_NONE = 0,
    PACKET_TYPE_REQUEST_FOCUS,
    PACKET_TYPE_RESPOND_FOCUS,
    PACKET_TYPE_REQUEST_CONTRAST_HISTOGRAM,
    PACKET_TYPE_RESPOND_CONTRAST_HISTOGRAM,
    PACKET_TYPE_REQUESTS
};

#endif
