#ifndef PROJECT_BYTETIDE_PEER_H
#define PROJECT_BYTETIDE_PEER_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config/config.h"

struct client_args {
    struct sockaddr_in server_addr;
};

void *start_server(void *arg);

void *start_client(void *arg);

#endif
