#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#define RESPONSE_BUFFER_SIZE (1024 * 1024 * 8)
#define REQUEST_BUFFER_SIZE (64 * 1024)
#define ELEMENT_FIELD_BUFFER_SIZE (256)

#include "web-server.h"

void *web_server_loop(void *arg)
{
    web_server_t *server = (web_server_t *) arg;

    char buffer[REQUEST_BUFFER_SIZE];
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>hello, unto you world</html>\r\n";

    /* Create a socket */
    server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sockfd == -1) {
        perror("webserver (socket)");
        return 0;
    }
    fprintf(stderr, "socket %d created\n", server->sockfd);

    /* create address to bind socket to */
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof (host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(server->port);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* create client address */
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    /* bind socket to address */
    if (bind(server->sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        perror("webserver (bind)");
        return 0;
    }
    printf("bind(): success\n");

    /* Listen for incoming connections */
    if (listen(server->sockfd, SOMAXCONN) != 0) {
        perror("listen(): failure\n");
        return 0;
    }
    printf("server listening for connections\n");

    while (server->run) {
        /* accept incoming connections */
        int fd = accept(server->sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (fd < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        /* get client address */
        int sockn = getsockname(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getsockname)");
            continue;
        }

        /* read incoming request from socket */
        int valread = read(fd, buffer, REQUEST_BUFFER_SIZE);
        if (valread < 0) {
            perror("webserver (read)");
            continue;
        }

        /* read request elements */
        char method[ELEMENT_FIELD_BUFFER_SIZE], uri[ELEMENT_FIELD_BUFFER_SIZE], version[ELEMENT_FIELD_BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, version, uri);

        /* respond to socket */
        int valwrite = write(fd, resp, strlen(resp));
        if (valwrite < 0) {
            perror("webserver (write)");
            continue;
        }

        close(fd);
    }

    return 0;
}

int start_web_server(int port)
{

}