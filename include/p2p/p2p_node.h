#ifndef P2P_NODE_H
#define P2P_NODE_H

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "p2p/peer.h"
#include "p2p/package.h"
#include "net/packet.h"

struct server_args {
    int max_peers;
    u_int16_t port;
    struct peer_list *peer_list;
};

struct client_handler_args {
    struct peer new_peer;
    struct peer_list *peer_list;
};

struct client_args {
    char ip[MAX_IP_SIZE];
    uint16_t port;
    struct peer_list *peer_list;
};

struct client_handler_args *create_chandler_args(int peer_fd, char *peer_ip,
        uint16_t peer_port, struct peer_list *peer_list);

struct client_args *create_client_args(char *ip, uint16_t port, struct peer_list *peer_list);

void *start_server(void *args);

void *start_client(void *arg);

#endif
