#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <pthread.h>

typedef struct {
    int sockfd;
    int port;
    int run;
    pthread_t thread_id;
} web_server_t;

int start_web_server(int port);

#endif

