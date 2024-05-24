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

struct client_handler_args {
    struct peer new_peer;
    struct peer_list *peer_list;
    struct package_list *package_list;
};

struct server_args {
    int max_peers;
    u_int16_t port;
    struct peer_list *peer_list;
    struct package_list *package_list;
};

struct client_args {
    char ip[MAX_IP_SIZE];
    uint16_t port;
    struct peer_list *peer_list;
    struct package_list *package_list;
};

/**
 * Start a server thread to listen connection requests
 * @param args
 * @return
 */
void *start_server(void *args);

struct client_args *create_client_args(char *ip, uint16_t port, struct
        peer_list *peer_list, struct package_list *package_list);

/**
 * Start a client thread to connect to a new peer
 * @param arg struct client_args type
 * @return
 */
void *start_client(void *arg);

#endif
