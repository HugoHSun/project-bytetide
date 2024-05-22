#ifndef P2P_NODE_H
#define P2P_NODE_H

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "p2p/peer.h"
#include "p2p/package.h"
#include "net/packet.h"

struct server_args {
    int max_peers;
    u_int16_t port;
    struct peer_list *peer_list;
};

struct client_args {
    struct sockaddr_in server_addr;
};

void *start_server(void *args);

void *start_client(void *arg);

#endif
