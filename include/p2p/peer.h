#ifndef PEER_H
#define PEER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_IP_SIZE 16

struct peer {
    int peer_fd; // -1 indicates non-existence
    char peer_ip[MAX_IP_SIZE];
    u_int16_t peer_port;
};

struct peer_list {
    int max_size;
    int num_peers;
    struct peer *peers;
};

struct peer_list *create_peer_list();

void add_peer(struct peer_list *list, struct peer new_peer);

void remove_peer(struct peer_list *list, struct peer peer);

void print_peer_list(struct peer_list *list);

void free_peer_list(struct peer_list *list);

#endif
